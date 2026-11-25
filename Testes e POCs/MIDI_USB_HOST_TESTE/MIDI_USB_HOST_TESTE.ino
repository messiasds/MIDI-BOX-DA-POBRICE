/***

Objetivos:
Conectar MSC Mixer Mvave e o roland juno di através de um hub USB;
Enviar um SysEX RQ1 quando houver troca de patch no juno-di
Acender LEDs do mixer baseado na reposta do Rq1; 

TODO Rotear controles de volume solo e mute ao juno-di
*/


#include "EspUsbHost.h"
#include <vector>

std::vector<uint8_t> sysexBuffer;
bool gravando = false;

EspUsbHost usb;
uint8_t juno_di;
uint8_t mixer_midi;
EspUsbHostDeviceInfo junodi_device;
EspUsbHostDeviceInfo mixer_device;

static const char *const NOTE_NAMES[] = {
  "C", "C#", "D", "D#", "E", "F",
  "F#", "G", "G#", "A", "A#", "B"
};

// syEX RQ1 juno di - retorna informações(syEx) das 16 partes da performance que está selecionada no momento
const uint8_t sysExPerformInfo[] = { 0xF0, 0x41, 0x10, 0x00, 0x00, 0x3A, 0x11, 0x10, 0x00, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x40, 0xF7 };

static void printNote(uint8_t note) {
  Serial.printf("%s%d", NOTE_NAMES[note % 12], (int)(note / 12) - 1);
}

static void printMidiMessage(const EspUsbHostMidiMessage &msg) {
  // en: Decode the high nibble as the MIDI message type and the low nibble as channel.
  // ja: 上位4bitをMIDIメッセージ種別、下位4bitをチャンネルとして解釈します。
  uint8_t type = msg.status & 0xF0;
  uint8_t channel = (msg.status & 0x0F) + 1;

  switch (type) {
    case 0x80:
      Serial.printf("Note Off   ch=%u note=", channel);
      printNote(msg.data1);
      Serial.printf(" vel=%u\n", msg.data2);
      break;
    case 0x90:
      Serial.printf("Note On    ch=%u note=", channel);
      printNote(msg.data1);
      Serial.printf(" vel=%u\n", msg.data2);
      break;
    case 0xA0:
      Serial.printf("Aftertouch ch=%u note=", channel);
      printNote(msg.data1);
      Serial.printf(" pressure=%u\n", msg.data2);
      break;
    case 0xB0:
      Serial.printf("CC         ch=%u ctrl=%u val=%u\n", channel, msg.data1, msg.data2);
      break;
    case 0xC0:
      Serial.printf("Program    ch=%u prog=%u\n", channel, msg.data1);
      break;
    case 0xD0:
      Serial.printf("Pressure   ch=%u val=%u\n", channel, msg.data1);
      break;
    case 0xE0:
      Serial.printf("PitchBend  ch=%u val=%d\n", channel,
                    (int)(((uint16_t)msg.data2 << 7) | msg.data1) - 8192);
      break;
    default:
      handleSysExRoland(msg);
      break;
  }
}

static void handleMidiMessage(const EspUsbHostMidiMessage &msg) {

  uint8_t tipo = msg.status & 0xF0;         // extrai o codigo midi do byte
  uint8_t canal = (msg.status & 0x0F) + 1;  // extrai o canal do byte
  uint8_t device = msg.address;


  if (device == junodi_device.address) {
    switch (tipo) {
      case 0xC0:
        Serial.println("Enviar Syex ao juno di");
        usb.midiSendSysEx(sysExPerformInfo, sizeof(sysExPerformInfo), junodi_device.address);
    }
  }
}

// Essa lib manda a mensagem SySex quebradas em 3 bytes pois nao trata sysex proprietário
static void handleSysExRoland(const EspUsbHostMidiMessage &mensagem) {

  uint8_t temp[] = { mensagem.status, mensagem.data1, mensagem.data2 };

  for (const auto &msg : temp)
    if (msg == 0xF0) {
      sysexBuffer.clear();
      sysexBuffer.reserve(512);
      sysexBuffer.push_back(msg);
      gravando = true;
    } else if (gravando) {
      if (msg == 0xF7) {
        sysexBuffer.push_back(msg);
        sysexBuffer.shrink_to_fit();
        gravando = false;
        acender_track_led();

      } else if (!(msg & 0x80)) {  // Só entra se for dado puro (0x00 a 0x7F)
        sysexBuffer.push_back(msg);
      }
    }
}

void acender_track_led() {

  uint8_t parte = sysexBuffer[11];

  for (const auto &sysex : sysexBuffer) {
    Serial.printf("%X ", sysex);
  }
  Serial.println();
  // acendendo track led branco, byte 13 indica a parte (0 a 15)
  if (sysexBuffer[12]) {
    usb.midiSendNoteOn(1, 24 + parte, 127, mixer_device.address);  //incrementa o botao a partir do C1 + parte atual
  } else {
    usb.midiSendNoteOn(1, 24 + parte, 0, mixer_device.address);
  }
}

void setup() {

  Serial.begin(115200);
  delay(500);

  Serial.println("Aguardando conexões usb");

  //en: Register the connection notification before startup; the reported address belongs to this enumeration.
  usb.onDeviceConnected(
    [](const EspUsbHostDeviceInfo &device) {
      Serial.print("connected: ");
      espUsbHostPrint(device);

      if (device.vid == 0x4353) {
        mixer_midi = device.address;
        mixer_device = device;
        Serial.println("MSC MIXER CONECTADO!");
      } else if (device.vid == 0x0582) {
        junodi_device = device;
        Serial.println("JUNO DI CONECTADO!");
      } else {
        Serial.println("OUTRO DISPOSITIVO");
      }
    });

  // en: Use disconnect notifications to invalidate application state; a reconnect can have a new address.
  usb.onDeviceDisconnected([](const EspUsbHostDeviceInfo &device) {
    Serial.print("disconnected: ");
    espUsbHostPrint(device);
  });

  usb.onMidiMessage([](const EspUsbHostMidiMessage &msg) {
    printMidiMessage(msg);
    handleMidiMessage(msg);
  });

  // en: Start host processing; success does not mean a peripheral has finished enumeration.
  if (!usb.begin()) {
    Serial.printf("usb.begin() failed: %s\n", usb.lastErrorName());
  }
}

void loop() {

  // put your main code here, to run repeatedly:
}
