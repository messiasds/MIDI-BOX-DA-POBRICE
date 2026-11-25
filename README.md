# MIDI-BOX-DA-POBRICE

## Objetivos 
Desenvolvimento de um mini processador MIDI utilizando uma placa ESP32-S3, com o objetivo de receber comandos de uma M-VAVE SMC Mixer, processá-los e retransmiti-los ao teclado Roland Juno-Di e vice-versa.

Criar uma solução MIDI personalizada, expandindo as possibilidades de controle do teclado com a SMC Mixer sem precisar pagar 500 contos pra importar da China! kk 

## Motivação 
usar C/C++ pra algo útil além de printf() kkk e não gastar 500 pilas;

## Hardware
Placa ESP32-S3 

## Funcionalidades
- acender e apagar LEDs com base em cada parte ativa da performance selecionada no Juno-di (usar DAW mode, CC Mode não da pra controlar todos os LEDS);
- controlar SOLO E MUTE de cada parte da performance no Juno-Di;
- Manter volumes já selecionados nos Faders ao trocar de Patch;

## Abordagem
- Usar o sysEx REQ1 para solicitar ao Juno-Di informações sobre a performance na memória usar o retorno no MIDOUT para verificar informações sobre a performance selecionada;
- Ligar LEDS do SMC Mixer baseado nas partes ativas da performance no Juno-Di;
- Usar SysEx para enviar MUTE e SOLO para uma Parte da Performance no Juno-Di;
- Utilizar roteamento BLE --> MIDI 5 pinos (serial)
- Utilizar roteamento USB HOST <---> MIDI 5 PINOS (serial)
- Utilizar roteamento USB <--> HUB USB <---> USB
