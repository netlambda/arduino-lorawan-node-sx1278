

## Hardware

- Arduino Uno
- Ai - Thinker Ra-02 LoRa Module

## Software

- PlatformIO
- https://github.com/dragino/arduino-lmic

## Setup

#### wire connections

| LoRa Ra-02 transceiver module | Arduino Uno Pin |      | LoRa Ra-02 transceiver module | Arduino Uno Pin |
| ----------------------------- | --------------- | ---- | ----------------------------- | --------------- |
| ANT                           | -               |      | GND                           | -               |
| GND                           | GND             |      | DIO5                          | -               |
| DIO3                          | -               |      | RESET                         | 5               |
| DIO4                          | -               |      | NSS                           | 10              |
| 3.3V                          | 3.3V            |      | SCK                           | 13              |
| DIO0                          | 2               |      | MOSI                          | 11              |
| DIO1                          | 3               |      | MISO                          | 12              |
| DIO2                          | -               |      | GND                           | -               |

#### Change the pin mapping.

```c++
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 5,
    .dio = {2, 3, LMIC_UNUSED_PIN},
}; 
```

