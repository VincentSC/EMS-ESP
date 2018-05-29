/*
 * Header file for EMS.cpp
 *
 * You shouldn't need to change much in this file
 */

#ifndef __EMS_H
#define __EMS_H

#include <Arduino.h>

// EMS IDs
#define EMS_ID_NONE 0x00   // Fixed - used as a dest in broadcast messages
#define EMS_ID_BOILER 0x08 // Fixed - also known as MC10.
#define EMS_ID_ME 0x0B     // Fixed - our device, hardcoded as "Service Key"

// Special EMS Telegram Types
#define EMS_TYPE_NONE 0x00 // none

#define EMS_TX_MAXBUFFERSIZE 128 // max size of the buffer. packets are 32 bits

/* EMS UART transfer status */
typedef enum {
    EMS_RX_IDLE,
    EMS_RX_ACTIVE // Rx package is being sent
} _EMS_RX_STATUS;

typedef enum {
    EMS_TX_IDLE,
    EMS_TX_PENDING, // got Tx package to send, waiting for next Poll to send
    EMS_TX_ACTIVE   // Tx package being sent, no break sent
} _EMS_TX_STATUS;

typedef enum {
    EMS_TX_NONE,
    EMS_TX_READ,    // doing a read request
    EMS_TX_WRITE,   // doing a write request
    EMS_TX_VALIDATE // do a validate after a write
} _EMS_TX_ACTION;

// status/counters since last power on
typedef struct {
    _EMS_RX_STATUS emsRxStatus;
    _EMS_TX_STATUS emsTxStatus;
    uint16_t       emsRxPgks;            // received
    uint16_t       emsTxPkgs;            // sent
    uint16_t       emxCrcErr;            // CRC errors
    bool           emsPollEnabled;       // flag enable the response to poll messages
    bool           emsThermostatEnabled; // if there is a RCxx thermostat active
    bool           emsLogVerbose;        // Verbose logging
    unsigned long  emsLastPoll;          // in ms, last time we received a poll
    unsigned long  emsLastRx;            // timings
    unsigned long  emsLastTx;            // timings
    bool           emsRefreshed;         // fresh data, needs to be pushed out to MQTT
} _EMS_Sys_Status;

// The Tx send package
typedef struct {
    _EMS_TX_ACTION action; // read or write
    uint8_t        dest;
    uint8_t        type;
    uint8_t        offset;
    uint8_t        length;
    uint8_t        checkValue;    // value to validate against
    uint8_t        type_validate; // type to call after a successful Write command
    uint8_t        data[EMS_TX_MAXBUFFERSIZE];
} _EMS_TxTelegram;

/*
 * Telegram package defintions
 */
typedef struct {
    // UBAParameterWW
    bool    wWActivated;   // Warm Water activated
    uint8_t wWSelTemp;     // Warm Water selected temperature
    bool    wWCircPump;    // Warm Water circulation pump Available
    uint8_t wWDesiredTemp; // Warm Water desired temperature

    // UBAMonitorFast
    uint8_t selFlowTemp; // Selected flow temperature
    float   curFlowTemp; // Current flow temperature
    float   retTemp;     // Return temperature
    bool    burnGas;     // Gas on/off
    bool    fanWork;     // Fan on/off
    bool    ignWork;     // Ignition on/off
    bool    heatPmp;     // Circulating pump on/off
    bool    wWHeat;      // 3-way valve on WW
    bool    wWCirc;      // Circulation on/off
    uint8_t selBurnPow;  // Burner max power
    uint8_t curBurnPow;  // Burner current power
    float   flameCurr;   // Flame current in micro amps
    float   sysPress;    // System pressure

    // UBAMonitorSlow
    float    extTemp;     // Outside temperature
    float    boilTemp;    // Boiler temperature
    uint8_t  pumpMod;     // Pump modulation
    uint16_t burnStarts;  // # burner restarts
    uint16_t burnWorkMin; // Total burner operating time
    uint16_t heatWorkMin; // Total heat operating time

    // UBAMonitorWWMessage
    float    wWCurTmp;  // Warm Water current temperature:
    uint32_t wWStarts;  // Warm Water # starts
    uint32_t wWWorkM;   // Warm Water # minutes
    bool     wWOneTime; // Warm Water one time function on/off
} _EMS_Boiler;

// RC20 data
typedef struct {
    float   setpoint_roomTemp;
    float   curr_roomTemp;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} _EMS_Thermostat;

// call back function signature
typedef bool (*EMS_processType_cb)(uint8_t * data, uint8_t length);

// Definition for each type, including the relative callback function
typedef struct {
    uint8_t            src;
    uint8_t            type;
    const char         typeString[50];
    uint8_t            size; // size of telegram, excluding the 4-byte header and crc
    EMS_processType_cb processType_cb;
} _EMS_Types;

// function definitions
extern void ems_parseTelegram(uint8_t * telegram, uint8_t len);
void        ems_init();
void        ems_doReadCommand(uint8_t type);
void        ems_setThermostatTemp(float temp);
void        ems_setWarmWaterTemp(uint8_t temperature);
void        ems_setWarmWaterActivated(bool activated);

void ems_setPoll(bool b);
bool ems_getPoll();
bool ems_getThermostatEnabled();
void ems_setThermostatEnabled(bool b);
bool ems_getLogVerbose();
void ems_setLogVerbose(bool b);

// private functions
uint8_t _crcCalculator(uint8_t * data, uint8_t len);
void    _processType(uint8_t * telegram, uint8_t length);
void    _initTxBuffer();
void    _buildTxTelegram(uint8_t data_value);
void    _debugPrintPackage(const char * prefix, uint8_t * data, uint8_t len, const char * color);

// helper functions
float    _toFloat(uint8_t i, uint8_t * data);
uint16_t _toLong(uint8_t i, uint8_t * data);
char *   _float_to_char(char * a, float f, uint8_t precision = 1);

// global so can referenced in other classes
extern _EMS_Sys_Status EMS_Sys_Status;
extern _EMS_TxTelegram EMS_TxTelegram;
extern _EMS_Boiler     EMS_Boiler;
extern _EMS_Thermostat EMS_Thermostat;

#endif
