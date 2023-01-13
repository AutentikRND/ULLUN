#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <strings_en.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
//#include "diawan.h"
#include "Nextion.h"
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Adafruit_SSD1306.h>
#include <RBDdimmer.h>
#include "RTClib.h"
#include <FS.h>
#include "SPIFFS.h"

#define DEBUG 0
#define endScreen
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = { "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu" };
String name;
const int OutPin = 32;   // wind sensor analog pin  hooked up to Wind P sensor "OUT" pin
const int TempPin = 35;  // temp sesnsor analog pin hooked up to Wind P sensor "TMP" pin
unsigned long previousMillis = 0;
unsigned long previousMillis1 = 0;
const int R1 = 33;  //This is the Arduino Pin that will control Relay #1
const int R2 = 27;  //This is the Arduino Pin that will control Relay #2
const int R3 = 23;  //This is the Arduino Pin that will control Relay #3
int qualwifi = 0;
int httpResponseCode = 200;
float WindB;
float tempC;
float offsite1 = 1, offsite2 = 1;
String suhu, kecepatanUdara;
double windMS;
float ZeroWind_V = 1.11;  // nilai ini yang di adjust

NexVariable va0 = NexVariable(0, 16, "va0");
NexVariable va1 = NexVariable(0, 17, "va1");
NexText t0 = NexText(0, 6, "t0");
NexText t1 = NexText(0, 7, "t1");
NexText t2 = NexText(0, 8, "t2");
NexText t3 = NexText(0, 9, "t3");
NexText t4 = NexText(0, 10, "t4");
NexText t5 = NexText(0, 12, "t5");
NexText t6 = NexText(0, 15, "t6");
NexSlider h0 = NexSlider(0, 1, "h0");
NexButton b0 = NexButton(0, 11, "b0");
NexDSButton bt0 = NexDSButton(0, 2, "bt0");
NexDSButton bt1 = NexDSButton(0, 3, "bt1");
NexDSButton bt2 = NexDSButton(0, 4, "bt2");

unsigned long mulai, selesai, dataStopWatch;
unsigned long mulai1, selesai1, dataStopWatch1;
unsigned long mulai2, selesai3, dataStopWatch4;
float jamtimer, menittimer, detiktimer, miliDetiktimer;
float jamtimer1, menittimer1, detiktimer1, miliDetiktimer1;
float jamtimer2, menittimer2, detiktimer2, miliDetiktimer2;
float jamtimerlast, menittimerlast, detiktimerlast;
float jamtimerlast1, menittimerlast1, detiktimerlast1;
//float offsite1 = 1, offsite2 = 1;
float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, regresiwind = 0, regresitemp = 0;
String serverName = "http://diawan.io/api/get_url";
String jamstop, detikstop, menitstop, waktustop, waktustop1;
String jamstop1, detikstop1, menitstop1;
String jamstop2, detikstop2, menitstop2;
String outgoing, last;
int i = 0, i1 = 0, i2 = 0, logic = 0, logic1 = 0;
int fPaus = 0, fPaus1 = 0, fPaus2 = 0;
long lastButton = 0, lastButton1 = 0, lastButton2 = 0;
long delayAntiBouncing = 50, delayAntiBouncing1 = 50, delayAntiBouncing2 = 50;
long dataPaus = 0, dataPaus1 = 0, dataPaus2 = 0;
long dataPauslast1, dataPauslast;
uint32_t lamp = 0;
uint32_t blow = 0;
uint32_t lampTL = 0;
uint32_t offall = 0;
uint32_t power = 0;
uint32_t resethepa = 0;
uint32_t resetlamp = 0;
//Parameters
const int zeroCrossPin = 34;
const int acdPin = 4;
int MIN_POWER = 0;
int MAX_POWER = 80;
int POWER_STEP = 10;
//Objects
dimmerLamp acd(acdPin, zeroCrossPin);
//diawan
WiFiManager wifiManager;

WiFiClient client;

//User ID
String userId = "c8879e6e-db31-44e4-905e-ee87f238076a";
//ID Device
String idDevice = "8762ac22-0df1-4b90-a7b2-a0bcf5ca2614";

//Email Account
String email = "talpha.autentik@gmail.com";
//Email Password
String pass = "MMzSRt";

//Parameter *parameter;
String linkdiawan;
int restart, reset;
bool portalRunning = false;
bool startAP = false;
unsigned int startTime = millis();

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;
//Nama WiFi Device>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char node_ID[] = "PR00112-1";

void setup() {
  Serial.begin(115200);
  nexInit(9600);
  acd.begin(NORMAL_MODE, ON);
  if (!SPIFFS.begin(true)) {
    debugln("An Error has occurred while mounting SPIFFS");
    return;
  }
  String datadaripaus = readFile(SPIFFS, "/datapaus.txt");
  dataPaus = atol(datadaripaus.c_str());
  String datadaripaus1 = readFile(SPIFFS, "/datapaus1.txt");
  dataPaus1 = atol(datadaripaus1.c_str());
  t2.setText(waktustop.c_str());
  t3.setText(waktustop1.c_str());
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);
  xTaskCreate(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1     /* Task handle to keep track of created task */
  );           /* pin task to core 0 */
  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreate(
    Task2code, /* Task function. */
    "Task2",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task2     /* Task handle to keep track of created task */
  );           /* pin task to core 1 */
  xTaskCreate(
    Task3code, /* Task function. */
    "Task3",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task3     /* Task handle to keep track of created task */
  );           /* pin task to core 1 */
}

void Task1code(void *pvParameters) {
  for (;;) {
    debug("Task1 running on core ");
    debugln(xPortGetCoreID());
    long start = millis();
    int tempRerata = analogRead(TempPin);
    for (int i = 0; i < 11; i++) {
      tempRerata += analogRead(TempPin);
      delay(10);
    }
    tempRerata /= 10;
    debug("tempRerata=");
    debugln(tempRerata);
    float tempRawAD = (tempRerata * 3.3) / 4096;
    float tempCraw = ((tempRawAD - 0.400) / .0195);
    regresitemp = (a * tempCraw) + (b * (pow(tempCraw, 2))) + c;
    tempC = (tempCraw + regresitemp) * offsite1;
    suhu = String(tempC, 1);
    debug("tempC=");
    debugln(tempC);
    int windADunits = analogRead(OutPin);
    float voltWind = (windADunits * 3.3) / 4096;
    float adjustmentV = voltWind - ZeroWind_V;
    if (adjustmentV < 0) {
      adjustmentV = 0;
    }
    float tempPow = pow(tempC, 0.115);
    float windMPH = pow(((adjustmentV / (3.038 * tempPow)) / 0.087), 3.009);  //rumus dari link kalibrasi
    //float windMPH = pow((((float)windADunits - 264.0) / 85.6814), 3.36814);
    float windMSraw = (windMPH * 0.44704);
    regresiwind = (d * windMSraw) + (e * (pow(windMSraw, 2))) + f;
    windMS = (windMSraw + regresiwind) * offsite2;
    kecepatanUdara = String(windMS, 2);
    debug("kecepatanUdara=");
    debugln(kecepatanUdara);
    Serial.println("TASK 1 TAKE TIME = ");
    Serial.println(millis() - start);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void Task2code(void *pvParameters) {
  for (;;) {
    debug("Task2 running on core ");
    debugln(xPortGetCoreID());
    long start = millis();
    debugln(power);
    if (power >= 90) {
      acd.setPower(90);
      debugln("LEBIHH");
    } else {
      acd.setPower(power - 10);
    }
    debugln(power);
    if (lamp == 1)  //When pressed dual state button dual_state =1
    {
      digitalWrite(R1, HIGH);
      selesai = millis();
      unsigned long over;
      // MATH time!!!
      dataStopWatch = selesai - mulai;
      dataStopWatch = dataPaus + dataStopWatch;
      jamtimer = int(dataStopWatch / 3600000);
      over = (dataStopWatch % 3600000);
      menittimer = int(over / 60000);
      over = (over % 60000);
      detiktimer = int(over / 1000);
      miliDetiktimer = over % 1000;
      debug(jamtimer);
      debug(":");
      debug(menittimer);
      debug(":");
      debug(detiktimer);
      debugln(".");
      jamstop = String(jamtimer, 0);
      menitstop = String(menittimer, 0);
      detikstop = String(detiktimer, 0);
      waktustop = jamstop + ":" + menitstop + ":" + detikstop;
      logic = 1;
      if (jamtimer < 10) {
        debugln(miliDetiktimer);
      }
      //    dataStopWatch = 0;
      //    dataPaus = 0;
    } else if (lamp == 0) {
      digitalWrite(R1, LOW);
      if ((millis() - lastButton) > delayAntiBouncing) {
        if (i == 0) {
          mulai = millis();
          fPaus = 0;
        } else if (i == 1) {
          if (logic == 1) {
            dataPaus = dataStopWatch;
            String pausstring = String(dataPaus);
            writeFile(SPIFFS, "/datapaus.txt", pausstring.c_str());
            logic = 0;
          }
          fPaus = 1;
        }
        i = !i;
      }
      lastButton = millis();
    }
    if (resetlamp == 1) {
      writeFile(SPIFFS, "/datapaus.txt", 0);
      dataPaus = 0;
    } else {
    }
    if (blow == 1)  //When pressed dual state button dual_state =1
    {
      digitalWrite(R2, HIGH);
      selesai1 = millis();
      unsigned long over1;
      // MATH time!!!
      dataStopWatch1 = selesai1 - mulai1;
      dataStopWatch1 = dataPaus1 + dataStopWatch1;
      jamtimer1 = int(dataStopWatch1 / 3600000);
      over1 = (dataStopWatch1 % 3600000);
      menittimer1 = int(over1 / 60000);
      over1 = (over1 % 60000);
      detiktimer1 = int(over1 / 1000);
      miliDetiktimer1 = over1 % 1000;
      debug(jamtimer1);
      debug(":");
      debug(menittimer1);
      debug(":");
      debug(detiktimer1);
      debugln(".");
      jamstop1 = String(jamtimer1, 0);
      menitstop1 = String(menittimer1, 0);
      detikstop1 = String(detiktimer1, 0);
      waktustop1 = jamstop1 + ":" + menitstop1 + ":" + detikstop1;
      logic1 = 1;
      if (jamtimer1 < 10) {
        debugln(miliDetiktimer1);
      }
    } else if (blow == 0) {
      digitalWrite(R2, LOW);
      if ((millis() - lastButton1) > delayAntiBouncing1) {
        if (i1 == 0) {
          mulai1 = millis();
          fPaus1 = 0;
        } else if (i1 == 1) {
          if (logic1 == 1) {
            dataPaus1 = dataStopWatch1;
            String pausstring1 = String(dataPaus1);
            writeFile(SPIFFS, "/datapaus1.txt", pausstring1.c_str());
            logic1 = 0;
          }
          fPaus1 = 1;
        }
        i1 = !i1;
      }
      lastButton = millis();
    }
    if (resethepa == 1) {
      writeFile(SPIFFS, "/datapaus1.txt", 0);
      dataPaus1 = 0;
    } else {
    }
    if (lampTL == 1)  //When pressed dual state button dual_state =1
    {
      digitalWrite(R3, HIGH);
    } else if (lampTL == 0) {
      digitalWrite(R3, LOW);
    }
    Serial.println("TASK 2 TAKE TIME = ");
    Serial.println(millis() - start);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
void Task3code(void *pvParameters) {
  for (;;) {
    debug("Task3 running on core ");
    debugln(xPortGetCoreID());
    t1.setText(kecepatanUdara.c_str());
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    t0.setText(suhu.c_str());
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    t2.setText(waktustop.c_str());
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    t3.setText(waktustop1.c_str());
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    yield();
    va0.getValue(&resetlamp);
    va1.getValue(&resethepa);
    h0.getValue(&power);
    bt0.getValue(&blow);
    bt1.getValue(&lampTL);
    bt2.getValue(&lamp);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
void loop(void) {
}

String readFile(fs::FS &fs, const char *path) {
  //debugf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    debugln("- empty file or failed to open file");
    return String();
  }
  debugln("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  debugln(fileContent);
  return fileContent;
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  //debugf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    debugln("− failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    debugln("− file written");
  } else {
    debugln("− frite failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  //debugf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    debugln("− failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    debugln("− message appended");
  } else {
    debugln("− append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  //debugf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    debugln("− file renamed");
  } else {
    debugln("− rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path) {
  //debugf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    debugln("− file deleted");
  } else {
    debugln("− delete failed");
  }
}
