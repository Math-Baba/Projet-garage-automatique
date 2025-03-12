//auteur : Baba Mathieu
//date : 11/03/2025

#include <NewPing.h> //Bibliothèque ultrason
#include <Servo.h> //Bibliothèque servomoteur
#include <Adafruit_Fingerprint.h>  // Bibliothèque Adafruit pour le capteur d'empreintes
#include <SoftwareSerial.h> // Bibliothèque pour utiliser des ports série supplémentaires

//Ultrason
const byte TRIGGER_PIN_IN = 12; //Broche trigger du capteur intérieur
const byte ECHO_PIN_IN = 10; //Broche trigger du capteur intérieur
const unsigned int MAX_DISTANCE = 400; //distance maximale de mesure en cm
NewPing sonar_in(TRIGGER_PIN_IN, ECHO_PIN_IN, MAX_DISTANCE);

//Servomoteur
Servo myservo;

//Led RGB
int redPin;
int greenPin;
int bluePin;

//Fingerprint
// Définition des pins RX et TX pour le capteur d'empreintes
SoftwareSerial mySerial(A1, A0);  // RX sur pin A1, TX sur pin A0
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);  // Création de l'objet pour le capteur

void setup() {
  Serial.begin(9600);
  mySerial.begin(57600);// Initialisation de la communication avec le capteur à 57600 bauds
  myservo.attach(9);
  myservo.write(0);

  redPin=3;
  greenPin=6;
  bluePin=5;

  // Vérification de la connexion avec le capteur fingerprint sensor
  finger.begin(57600); 
  Serial.println("Capteur d'empreintes détecté !");

  // Enregistrer l'empreinte
  enregistrerEmpreinte(1);  // Enregistre l'empreinte avec ID = 1

}

void loop() {
  
  //Par défaut la led est rouge
  analogWrite(redPin, 0);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 255); 

  //Détection d'une empreinte
  detecterEmpreinte();

  //Ouverture de la porte si une empreinte est valide
  ouvertureGarage();

  //Fermeture du garage
  fermetureGarage();

  //Attente de sortie de la voiture
  sortieGarage();
}






//Fonction d'enregistrement d'une empreinte
void enregistrerEmpreinte(int id) {
  Serial.print("Enregistrement de l'empreinte ID : ");
  Serial.println(id);

  // Attente pour la capture de l'empreinte
  while (finger.getImage() != FINGERPRINT_OK);
  finger.image2Tz(1); 
  Serial.println("Retirez votre doigt.");
  delay(2000);

  // Attente pour la deuxième capture de l'empreinte
  while (finger.getImage() != FINGERPRINT_NOFINGER);
  Serial.println("Placez le même doigt à nouveau.");
  while (finger.getImage() != FINGERPRINT_OK);
  finger.image2Tz(2);  

  // Création du modèle d'empreinte et enregistrement
  if (finger.createModel() == FINGERPRINT_OK) {
    if (finger.storeModel(id) == FINGERPRINT_OK) {
      Serial.println("Empreinte enregistrée avec succès !");
    } else {
      Serial.println("Échec de l'enregistrement.");
    }
  } else {
    Serial.println("Les empreintes ne correspondent pas.");
  }
}

//Fonction de détection d'une empreinte
void detecterEmpreinte() {
  Serial.println("Placez votre doigt sur le capteur...");

  // Boucle infinie jusqu'à reconnaissance d'une empreinte
  while (true) {  
    if (finger.getImage() == FINGERPRINT_OK) {
      Serial.println("Empreinte détectée !");
      
      if (finger.image2Tz() == FINGERPRINT_OK) {
        Serial.println("Conversion réussie !");
        
        if (finger.fingerFastSearch() == FINGERPRINT_OK) {
          Serial.print("Empreinte reconnue, ID : ");
          Serial.println(finger.fingerID);
          
          if (finger.fingerID == 1) {  // Si l'ID correspond
            Serial.println("Accès autorisé !");
            return;  // Sort de la fonction et permet au reste du programme de s'exécuter
          } else {
            Serial.println("Accès refusé !");
          }
        } else {
          Serial.println("Empreinte inconnue !");
        }
      } else {
        Serial.println("Erreur lors de la conversion de l'empreinte !");
      }
    }
    
    delay(500); 
  }
}

// Fonction pour attendre que la voiture soit à l'intérieur avant de fermer la porte
void fermetureGarage() {
  unsigned int distance_in = sonar_in.ping_cm();  // Mesure la distance

  // Boucle jusqu'à ce que la voiture soit à l'intérieur
  while (distance_in > 15 || distance_in == 0) { 
    Serial.print("Attente voiture à l'intérieur, distance : ");
    Serial.println(distance_in);
    distance_in = sonar_in.ping_cm();  
    delay(100);  
  }
  
  // Une fois la voiture à l'intérieur, on ferme la porte
  analogWrite(redPin, 0);
  analogWrite(greenPin, 165);
  analogWrite(bluePin, 255);
  for(int pos = 90; pos >= 0; pos--) {
    myservo.write(pos);
    delay(10);
  }
  delay(3000);
  
  //Couleur rouge
  analogWrite(redPin, 0);
  analogWrite(bluePin, 255);
  analogWrite(greenPin, 255);
  delay(3000);
}

//Fonction d'ouverture du garage
void ouvertureGarage(){
  for(int pos=0; pos<=90; pos++){
      myservo.write(pos);
      delay(10);
      //Couleur orange
      analogWrite(redPin, 0);
      analogWrite(greenPin, 165);
      analogWrite(bluePin, 255);
  }
  //Couleur verte
  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 255);
}

//Fonction de sortie du véhicule
void sortieGarage() {
  Serial.println("Attente de la sortie du véhicule...");

  // Attente que la voiture soit détectée à l'intérieur (distance < 15 cm)
  while (sonar_in.ping_cm() <= 15 && sonar_in.ping_cm() > 0) {
    delay(100);
  }

  Serial.println("Voiture en mouvement ! Ouverture de la porte...");
  ouvertureGarage(); 

  
  while (sonar_in.ping_cm() < 50 && sonar_in.ping_cm() > 0) {
    Serial.println("Véhicule en train de sortir...");
    delay(500);
  }

  fermetureGarage();  
}

