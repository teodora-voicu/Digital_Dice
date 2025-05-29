/*
 *  DIGITAL DICE  - Arduino UNO
 *  — Shake → pornește DICE.WAV pe pin 9
 *  — Stop Shake → afișează două numere pe OLED
 *
 *  HARDWARE:
 *    LIS2DH12 (I²C)        | SDA/SCL = A4/A5
 *    SSD1306 OLED 128×64   | SDA/SCL = A4/A5
 *    Micro-SD (SPI)        | CS = 10
 *    Buzzer pasiv          | pin 9 (Timer1 PWM)
 */

#include <Wire.h> // permite comunicarea I2C
#include "SparkFun_LIS2DH12.h" // driverul pentru accelerometru
#include <SPI.h> // pentru SD (comunicarea SPI)
#include <SD.h> // pt citire/scrie fisiere pe cardul SD
#include <TMRpcm.h> //  redarea fisierelor WAV mono printr-un PWM pe buzzer
#include <U8g2lib.h> // desene pe OLED

#define SD_CS_PIN        10 // chip select pt modulul SD
#define SPEAKER_PIN       9 // buzzerul pasiv conectat pe pinul 9 (pwm timer)

#define SHAKE_THRESHOLD 2000     // daca diferenta de citiri depaseste pragul e considerat shake
#define SHAKE_TIMEOUT    300     // ms fără miscare = aruncare terminata
#define RESULT_HOLD     1500     // ms pastram rez pe ecran inainte sa schimbam iar la Ready

const char WAV_NAME[] = "DICE.WAV";

SPARKFUN_LIS2DH12 accel; // cream accelerometrul
TMRpcm             audio; // playerul care genereaza pwm ul pt a reda fisierul wav pe buzzer
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE); // obiectul care controleaza ecranul

enum class Stare : uint8_t { GATA, SHAKING, REZULTAT }; // logicile posibile
Stare stareCurenta = Stare::GATA; // starea de start

unsigned long tStartStare; // mom cand s-a schimbat ultima data starea
int16_t lastX, lastY, lastZ; // valorile de pe fiecare axa a accelerometrului


// primeste un sir si un font, si scrie acel text centrat orizontal la linia y pe ecran
void drawCentered(const char* text, uint8_t y, const uint8_t* font) {
  u8g2.setFont(font); // font select
  int w = u8g2.getStrWidth(text); // latime txt
  u8g2.firstPage();  do { u8g2.drawStr((128 - w) / 2, y, text); } while (u8g2.nextPage()); // plaseaza incep in centrul ecranului
}

void setup() {
  // Serial.begin(9600); // pt debug
  if (!accel.begin()) while (1);   // start accelerometru si se blocheaza daca lipseste
  u8g2.begin(); // initializeaza ecranul
  drawCentered("READY!", 32, u8g2_font_ncenB14_tr); // afis ready pt inceput

  audio.speakerPin = SPEAKER_PIN; // spune playerului pe ce pin scoate semnal pwm catre buzzer
  audio.CSPin      = SD_CS_PIN; // ce pin e chipselect pt sd

  if (!SD.begin(SD_CS_PIN))  while (1);    // stop dacă SD fail
  if (!SD.exists(WAV_NAME))  while (1);    // stop dacă lipseste DICE.wav

  audio.setVolume(5);          // 0-6, 5 a fost optim
  audio.quality(1);            // pt sunet mai clar

  randomSeed(analogRead(A0)); // foloseste zgomot de pe pinul A0 ca baza pt generarea numerelor aleatoare
  lastX = accel.getRawX();  lastY = accel.getRawY();  lastZ = accel.getRawZ(); // citeste si salveaza primele valori de la accelro
}

void loop() {
  int16_t x = accel.getRawX(),  y = accel.getRawY(),  z = accel.getRawZ(); // cit acceleratia pe fiec axa
  int16_t dX = abs(x - lastX),  dY = abs(y - lastY),  dZ = abs(z - lastZ); // dif fata de ultima data
  lastX = x;  lastY = y;  lastZ = z; // actualizare valori

  switch (stareCurenta) { // algem ce stare urm in fct de starea curenta

  case Stare::GATA:
    if (dX > SHAKE_THRESHOLD || dY > SHAKE_THRESHOLD || dZ > SHAKE_THRESHOLD) { // am detectat miscarea
      stareCurenta = Stare::SHAKING; // deci trecem in starea shaking
      tStartStare  = millis();
      drawCentered("Shake!", 32, u8g2_font_ncenB14_tr); // afisam shake
      audio.play(WAV_NAME); // si redam sunetul DICE
    }
    break;

  case Stare::SHAKING:
    if (dX > SHAKE_THRESHOLD || dY > SHAKE_THRESHOLD || dZ > SHAKE_THRESHOLD) { // daca ince se scturua accelreo
      tStartStare = millis();                 // resetam timeoutul
    }
    if (millis() - tStartStare > SHAKE_TIMEOUT) {   // daca de la ultima miscare a trecut mai mult de shake timeout
      audio.stopPlayback();                   // oprim sunetul
      uint8_t d1 = random(1,7), d2 = random(1,7); // afisam 2 nre random
      char buf[6];  snprintf(buf, sizeof(buf), "%d %d", d1, d2);
      drawCentered(buf, 50, u8g2_font_logisoso32_tn);

      stareCurenta = Stare::REZULTAT; // trecem la starea de result
      tStartStare  = millis();
    }
    break;

  case Stare::REZULTAT:
    if (millis() - tStartStare > RESULT_HOLD) { // daca a trecut timpul de cat tb tinut rez pe ecran
      drawCentered("READY!", 32, u8g2_font_ncenB14_tr); // afisam din nou starea de ready pt urmatoare miscare si
      stareCurenta = Stare::GATA; // actualizam si starea
    }
    break;
  }

  delay(5);                 // pauza pt CPU
}