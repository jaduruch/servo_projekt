/*********************************************
* Servo Projekt 
* Jana Ruch
* 22.11.2023
* v9
*********************************************/
#include <MsTimer2.h>                                    //Timer Libary für Interrupt hinzufügen

//Pin Definitionen
#define SERVO 8                                          //Pinnummer des Servo-Motors (Oragnge Leitung)
#define potPin A0                                        //Pinnummer des Potis an Arduino
#define BIT_0 10                                         //Pinnummer des Tasters BIT_0
#define BIT_1 11                                         //Pinnummer des Tasters BIT_1
#define BIT_2 12                                         //Pinnummer des Tasters BIT_2
#define BIT_3 13                                         //Pinnummer des Tasters BIT_3

//Servo Definitionen
#define T_in_MIN 500                                     //0°
#define T_in_MAX 2500                                    //180°
#define frequenz 50                                      //in Hz

int POSITIONS[] = {10, 20, 40, 60, 80, 100, 120, 140, 160, 180};

//Definitionen 
#define LEFT !digitalRead(BIT_0)                         //Taster LEFT einlesen
#define RIGHT !digitalRead(BIT_1)                        //Taster RIGHT einlesen
#define SAVE !digitalRead(BIT_2)                         //Taster SAVE einlesen
#define PLAY !digitalRead(BIT_3)                         //Taster PLAY einlesen
#define potVal analogRead(potPin)                        //Poti einlesen
#define period 1000/frequenz                             //Periodenzeit berechnen

//Globale Variabeln
char Eingabe = 'a';                                      //Variable für eingegebne Aufgabe
char Aufgabe;                                            //Variable um Aufgabe abzuspeichern
int T_in = 0;                                            //Variable um berechnete einschaltzeit des PWMs abzuspeichern
float angle = 0;                                         //Variable welche den Aktuellen Winkel speichert
int j = 0;                                               //Variable für array Position
float x = 1;                                             //Variable um Geschwindigkeit zu erhöhen
long startTime = 0;                                      //Variable für Zeiterkennung

//Variabeln für Flankenerkennung
bool LEFT_war;
bool RIGHT_war;
bool SAVE_war;
bool PLAY_war;
bool LEFT_switch = 0;

void setup() 
{
    pinMode(SERVO, OUTPUT);                              //SERVO als Ausgang definieren
    MsTimer2::set(period, setServo);                     //20ms Interrupt für 50Hz
    MsTimer2::start();                                   //Timer für Interrput starten
    pinMode(BIT_0, INPUT_PULLUP);                        //Pullup Widerstand an Pin von BIT_0 schalten
    pinMode(BIT_1, INPUT_PULLUP);                        //Pullup Widerstand an Pin von BIT_1 schalten
    pinMode(BIT_2, INPUT_PULLUP);                        //Pullup Widerstand an Pin von BIT_2 schalten
    pinMode(BIT_3, INPUT_PULLUP);                        //Pullup Widerstand an Pin von BIT_3 schalten
    Serial.begin(9600);                                  //Serieller Moitor starten
    Serial.println("Bitte Aufgabe eingeben");            //Text ausgeben
}

void loop() 
{
  if (Serial.available() > 0)                            //Überprüfe, ob Daten verfügbar sind
  {  
    Eingabe = tolower(Serial.read());                    //Empfangenen Buchstaben einlesen und in Kleinbuchstaben umwandeln
    
    if(Eingabe >= 'a' && Eingabe <= 'g')                 //Überprüfe ob eine gültige Aufgabe eingegeben wurde
    {
      Aufgabe = Eingabe;                                 //Schreibe den eingegebenen Buchstaben auf die Variable Aufgabe
      Serial.print("Aufgabe: ");
      Serial.println(Aufgabe);                           //Aktuelle Aufgabe in Seriellem Monitor ausgeben
      x = 0;                                             //Variablen für jede neu Aufgabe zurücksetzen
      j = 0;
    }
    if(Eingabe > 'g' && Eingabe <= 'z')                  //Falls Aufgabe ausserhalb des gültigen bereichs ist
    {
      Serial.println("Ungültige Eingabe, gib bitte eine Aufgabe von a bis g ein");
    }
    switch(Eingabe)                                      //Aufgaben erklärung
    {                                     
      case 'a':
        Serial.println("Beim Einschalten fährt der Servo auf 90°");
        break;
      case 'b':
        Serial.println("Mit Bit0 = 0 fährt der Servo auf 0°, mit Bit0 = 1 auf 180°");
        break;
      case 'c':
        Serial.println("Der Servo fährt 10 vorgegebene Zeigerstellungen an (array auf Zeile 22)");
        break;
      case 'd':
        Serial.println("Mit Bit_0 und Bit_1 wird die Zeigerstellung verändert. Dabei darf der Zeiger den Bereich 0°...180° nicht verlassen");
        break;
      case 'e':
        Serial.println("Wie Aufgabe d, aber nach Betätigung von Bit_0 oder Bit_1 ist die Drehgeschwindigkeit zuerst langsam und wird nach 1s zunehmend schneller.");
        break;
      case 'f':
        Serial.println("Wie Aufgabe e, jedoch gewünschte Zeigerstellung mit Bit_2 speichern. Es können max. 10 Zeigerstellungen gespeichert werden. Start der Robotertätigkeit mit Bit_3");
        break;
      case 'g':
        Serial.println("Servo Position kann mit Potentiometer eingestellt werden");
        break;
    }
  }
  
  switch(Aufgabe)                                        //Swich case für Aufgaben
  {
   case 'a':                                             //Servo fährt auf 90°
      angle = 90;                                        //Variable angle wird auf 90 gesetzt
    break;
    
    case 'b':                                            //Servo fährt entweder auf 0° oder 180°
      if(LEFT)                                           //Wenn Taster LEFT betätigt
        {
          angle = 180;                                   //winkel auf 180 setzen
        }else                                            //Wenn Taster LEFT nicht betätigt
        {
          angle = 0;                                     //winkel auf 0 setzen
        }
    break;
    
    case 'c':                                            //Servo fährt 10, in einem Array besimmten, Positionen an
      if(j < 10)                                         //Wenn j kleiner als 10 
      {
        angle = POSITIONS[j];                            //Winkel wird auf position j im Array gestellt
        j++;                                             //j inkrementieren
        delay(500);                                      //für 0,5s warten
      }

      if(PLAY > PLAY_war)                                //Flankenerkennung für Taster PLAY
      {
        j = 0;                                           //Roboterfunktion von vorne starten
      }
    break;
    
    case 'd':                                            //Servo Position kann mit zwei Tastern genau eingestellt werden
      if(LEFT && angle > 0)                              //Wenn Taster Links betätigt und Winkel grösser als 0° ist
        {
          angle --;                                      //Winkel dekrementieren (Servo nach Links bewegen)
        }
      if(RIGHT && angle < 180)                           //Wenn Taster Links betätigt und Winkel kleiner als 180° ist
        {
          angle ++;                                      //Winkel inkrementieren (Servo nach Rechts bewegen)
        }
    break;
    
    case 'e':
      steuerung();                                       //Funktion Steuerung aufrufen
    break;
    
    case 'f':
      steuerung();                                       //Funktion Steuerung aufrufen
      
      if(SAVE > SAVE_war)                                //Flankenerkennung für Taster SAVE
      {
        Serial.print("position ");                       //Position ausgeben
        Serial.print(j + 1);                             //plus 1 da array mit 0 beginnt
        Serial.println(" wird gespeichert");
        POSITIONS[j] = angle;                            //angle mit wert aus array übersreiben
        j++;                                             //j inkrementieren
      }
      
      if(j == 10)                                        //Wenn j = 10 ist
      {
        j = 0;                                           //j zurücksetzen
      }
      
      if(PLAY > PLAY_war)                                //Flankenerkennung für Taster PLAY
      {
        for(j = 0; j < 10; j++)                          //alle Positionen in array einmal abfahren
        {
          angle = POSITIONS[j];                          //Winkel wird auf position j im Array gestellt
          delay(500);                                    //für 0,5s warten
        }
      }
    break;
    
    case 'g':
      angle = map(potVal, 1023, 0, 0, 180);              //Winkel anhand von Poti berechnen
    break;      
  }
  
  LEFT_war = LEFT;                                       //Variabeln für Flankenerkennung mit Aktuellem Wert überschreiben
  RIGHT_war = RIGHT;
  SAVE_war = SAVE;
  PLAY_war = PLAY;
  delay(10);                                             //Programmunterbruch von 10ms
}

void setServo()                                          //wird alle 20ms Ausgeführt (50Hz)
{ 
    T_in = map(angle, 0, 180, T_in_MIN, T_in_MAX);       //Time-in anhand von Winkel berechnen
    digitalWrite(SERVO, HIGH);                           //PWM Signal erzeugen mit vorhin berechnetem Time-in
    delayMicroseconds(T_in);                             //für die Zeit von Time-in anhalten
    digitalWrite(SERVO, LOW);                            //PWM Signal auf LOW schalten
}

void steuerung ()                                        //Funktion für Steuerung von Aufgaben e und f
{       
  if(LEFT > LEFT_war || RIGHT > RIGHT_war)               //Flanken erkennung für Taster LEFT oder RIGHT
  {
    startTime = millis();                                //startTime auf millis setzen um Zeit zu Starten
  }

  if(LEFT)                                               //Wenn Taster LEFT gedrückt ist                                        
  {
    if(millis() - startTime < 1000)                      //vor 1s
    {
      angle -= 0.5;                                      //Winkel um 0.5 verkleinern
    }else                                                //ab 1s
    {
      angle -= x;                                        //Servo nach Links bewegen
      x += 0.15;                                         //Dreh geschwindigkeit erhöhen
    }
    if(angle < 0)                                        //Winkel kann nicht keiner als 0 sein
    {
      angle = 0;
    }
  }

  if(RIGHT)                                              //Wenn Taster RIGHT gedrückt ist                                      
  {
    if(millis() - startTime < 1000)                      //vor 1s
    {
      angle += 0.5;                                      //Winkel um 0.5 erhöhen
    }else                                                //ab 1s
    {
      angle += x;                                        //Servo nach Links bewegen
      x += 0.15;                                         //Dreh geschwindigkeit erhöhen
    }
    if(angle > 180)                                      //Winkel kann nicht grösser als 180 sein
    {
      angle = 180;
    }
  }
  
  if((LEFT | RIGHT) == 0)                                //Wenn LEFT und RIGHT nicht betätigt
  {
    x = 1;                                               //x zurücksetzen
  }
}
