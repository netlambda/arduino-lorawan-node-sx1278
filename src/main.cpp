/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the (early prototype version of) The Things Network.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in g1,
 *  0.1% in g2).
 *
 * Change DEVADDR to a unique address!
 * See http://thethingsnetwork.org/wiki/AddressSpace
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

static const PROGMEM u1_t NWKSKEY[16] = { 0xEF, 0xFA, 0xE5, 0xCB, 0x14, 0xEE, 0x8A, 0x9E, 0xB5, 0x46, 0xA4, 0x22, 0xCC, 0x61, 0x3E, 0xB7 };
static const u1_t PROGMEM APPSKEY[16] = { 0xAA, 0x21, 0x54, 0x75, 0x60, 0x4F, 0xB2, 0xCE, 0x37, 0x58, 0x26, 0xF7, 0x59, 0x18, 0x5A, 0x0A };
static const u4_t DEVADDR = 0x2601183D;

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui(u1_t *buf) {}
void os_getDevEui(u1_t *buf) {}
void os_getDevKey(u1_t *buf) {}

static uint8_t mydata[] = "Hello, world!!!";
static osjob_t initjob, sendjob, blinkjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;
const unsigned LED = 5;
const unsigned INDICATOR_LED = 4;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 5,
    .dio = {2, 3, LMIC_UNUSED_PIN},
};

#define LED_YELLOW 8
#define LED_GREEN 6

void do_send(osjob_t *j)
{
    // Payload to send (uplink)
    static uint8_t message[2];

    int value = analogRead(A0);
    message[0] = highByte(value);
    message[1] = lowByte(value);

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND)
    {
        Serial.println(F("OP_TXRXPEND, not sending"));
    }
    else
    {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Sending uplink packet..."));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent(ev_t ev)
{
    if (ev == EV_TXCOMPLETE)
    {
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));

        digitalWrite(INDICATOR_LED, HIGH);
        delay(200);
        digitalWrite(INDICATOR_LED, LOW);

        if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
              
        if (LMIC.dataLen)
        {
            Serial.println("Got data.................");
            int dimValue = LMIC.frame[LMIC.dataBeg];

            analogWrite(LED, dimValue);
    

            // for (int i = 0; i < LMIC.dataLen; i++) {
            //     // Serial.println(downlink[i]);
            //     Serial.println(LMIC.frame[i + LMIC.dataBeg], HEX);
            // }
        }
        // Schedule next transmission
        os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
    }

    if (ev == EV_RXCOMPLETE) {
        Serial.println(F("EV_RXCOMPLETE"));
        Serial.print("Got data.................");
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println(F("Starting"));

    //------ Added ----------------
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED, OUTPUT);
    pinMode(INDICATOR_LED, OUTPUT);

#ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
#endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

// Set static session parameters. Instead of dynamically establishing a session
// by joining the network, precomputed session parameters are be provided.
#ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession(0x1, DEVADDR, nwkskey, appskey);
#else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession(0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7, 14);

    // Start job
    do_send(&sendjob);
}

void loop()
{
    os_runloop_once();
    // do_send(&sendjob);
    // delay(60000);
}
