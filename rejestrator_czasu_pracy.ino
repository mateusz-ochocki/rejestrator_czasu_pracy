// Biblioteki do obsługi poszczególnych modułów
#include <MFRC522.h>  // Biblioteka dla RFID
#include <SPI.h>      // Biblioteka dla RFID i kart SD
#include <SD.h>       // Karty SD
#include <RTClib.h>   // Zegar

// Definicja pinów dla RFID
#define CS_RFID 10
#define RST_RFID 9

// Definicja pinu dla karty SD
#define CS_SD 4

// Utworzenie pliku do zapisywania odczytanych danych
File myFile;

// Instancja klasy dla RFID
MFRC522 rfid(CS_RFID, RST_RFID);

// Zmienna do przechowywania identyfikatora odczytanego z karty
String uidString;

// Instancja klasy dla RTC
RTC_DS1307 rtc;

// Zmienne do przechowywania czasu, kiedy użytkownik się zalogował
int userCheckInHour;
int userCheckInMinute;

// Piny dla diody LED i brzęczyka
const int LED = 6;
const int buzzer = 5;

void setup() {
  // Ustawienie brzęczyka i diody jako output
  pinMode(LED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Inicjalizacja portu szeregowego
  Serial.begin(9600);
  while (!Serial);  // Dla modeli Leonardo/Micro/Zero

  // Inicjalizacja magistrali SPI
  SPI.begin();

  // Inicjalizacja MFRC522
  rfid.PCD_Init();

  // Inicjalizacja karty SD
  Serial.print("Initializing SD card...");
  if (!SD.begin(CS_SD)) {
    Serial.println("Initialization failed!");
    return;
  }
  Serial.println("Initialization done.");

  // Inicjalizacja modułu RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  } else {
    // Ustawienie czasu RTC na datę i czas kompilacji skryptu
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
}

void loop() {
  // Sprawdzanie, czy została przyłożona nowa karta
  if (rfid.PICC_IsNewCardPresent()) {
    readRFID();
    logCard();
  }
  delay(10);
}

void readRFID() {
  rfid.PICC_ReadCardSerial();
  Serial.print("Tag UID: ");
  uidString = String(rfid.uid.uidByte[0]) + " " + String(rfid.uid.uidByte[1]) + " " +
               String(rfid.uid.uidByte[2]) + " " + String(rfid.uid.uidByte[3]);
  Serial.println(uidString);

  // Brzęczenie głośnika, jeśli została przyłożona karta
  tone(buzzer, 4000);
  delay(100);
  noTone(buzzer);
  delay(100);
  digitalWrite(6, HIGH);
  delay(1000);
  digitalWrite(6, LOW);
}

void logCard() {
  // Włączenie pinu wyboru karty SD
  digitalWrite(CS_SD, LOW);

  // Otwarcie pliku
  myFile = SD.open("DATA.txt", FILE_WRITE);

  // Zapis do pliku
  if (myFile) {
    Serial.println("File opened ok");
    myFile.print(uidString);
    myFile.print(", ");

    // Zapisanie czasu odczytu karty na karcie SD
    DateTime now = rtc.now();
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(',');
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.println(now.minute(), DEC);

    // Wypisanie czasu na magistrali serial
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.println(now.minute(), DEC);
    Serial.println("Successfully written on SD card");
    myFile.close();

    // Zapisanie o której godzinie została zarejestrowana karta
    userCheckInHour = now.hour();
    userCheckInMinute = now.minute();
  } else {
    Serial.println("Error opening DATA.txt");
  }

  // Wyłączenie karty SD po zapisaniu danych
  digitalWrite(CS_SD, HIGH);
}
