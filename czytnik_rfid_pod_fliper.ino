#include <SPI.h>
#include <MFRC522.h>

// --- NOWE PINY DLA ESP32-C3 ---
#define RST_PIN   9
#define SS_PIN    10
#define LED_PIN   8

// Definiujemy piny dla magistrali SPI
#define SCK_PIN   6
#define MISO_PIN  5
#define MOSI_PIN  7

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Tutaj wpisujemy UID Twojej głównej karty
byte authorizedUID[4] = {0xAB, 0x4E, 0x13, 0x00};

void setup() {
  Serial.begin(115200); 
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); 

  // KRYTYCZNA ZMIANA: Inicjalizacja SPI z jawnym podaniem pinów dla ESP32-C3!
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN); 
  mfrc522.PCD_Init();
  
  // Wzmocnienie anteny na maxa
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max); 
  mfrc522.PCD_AntennaOn(); 
  
  Serial.println("\n=== SYSTEM ZAMKA RFID (Gotowy) ===");
  Serial.println("Oczekuję na kartę autoryzowaną...");
  Serial.println("----------------------------------");
}

void loop() {
  // Sprawdź, czy przyłożono nową kartę
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  // Próba odczytania numeru seryjnego karty (UID)
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Wypisz UID przyłożonej karty/Flippera na ekran
  Serial.print("Odczytano UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // --- LOGIKA SPRAWDZAJĄCA DOSTĘP ---
  bool accessGranted = true;
  
  // Sprawdzamy, czy długość UID to standardowe 4 bajty
  if (mfrc522.uid.size == 4) {
    for (byte i = 0; i < 4; i++) {
      // Jeśli chociaż jeden z 4 bajtów się nie zgadza, zablokuj dostęp
      if (mfrc522.uid.uidByte[i] != authorizedUID[i]) {
        accessGranted = false;
        break;
      }
    }
  } else {
    // Odmowa, jeśli karta ma inny format (np. 7-bajtowy UID)
    accessGranted = false; 
  }

  // --- REAKCJA SYSTEMU ---
  if (accessGranted) {
    Serial.println(">>> DOSTĘP PRZYZNANY! Otwieranie zamka...");
    
    // Zaświeć diodę na zielono/jako znak otwarcia na 3 sekundy
    digitalWrite(LED_PIN, HIGH);
    delay(3000);
    digitalWrite(LED_PIN, LOW);
    
  } else {
    Serial.println(">>> ODMOWA DOSTĘPU! Nieznany tag.");
    
    // Zróbmy efekt "Odmowy" - szybkie 3 mignięcia diodą
    for(int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }
  
  Serial.println("----------------------------------");
  
  // Usypia kartę do czasu jej ponownego zbliżenia
  mfrc522.PICC_HaltA();
}
