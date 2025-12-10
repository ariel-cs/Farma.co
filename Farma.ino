#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

LiquidCrystal_I2C tela(0x27, 16, 2);

const byte QTD_LINHAS  = 4;
const byte QTD_COLUNAS = 4;

char mapaTeclas[QTD_LINHAS][QTD_COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pinosLinhas[QTD_LINHAS]   = {9, 8, 7, 6};
byte pinosColunas[QTD_COLUNAS] = {5, 4, 3, 2};

Keypad teclado = Keypad(
  makeKeymap(mapaTeclas),
  pinosLinhas,
  pinosColunas,
  QTD_LINHAS,
  QTD_COLUNAS
);

SoftwareSerial portaLeitor(12, 13);

Servo servoEsquerdo;
Servo servoDireito;

const int pinoServoEsquerdo = 10;
const int pinoServoDireito  = 11;

const int anguloFechado = 0;
const int anguloAberto  = 80;

enum EstadoSistema {
  ESPERANDO_CPF,
  LEITOR_ATIVO
};

EstadoSistema estadoAtual = ESPERANDO_CPF;

// ========== LISTA DE CPFs AUTORIZADOS ==========
const int QTD_CPFS = 7;  // Altere este número conforme necessário
const String CPFS_AUTORIZADOS[QTD_CPFS] = {
  "12345678912",
  "08981397406",
  "13315566437",
  "98765432109",
  "11122233344",
  "07712603006",
  "83909222072"
};

const byte TAMANHO_MAX_CPF = 11;
String cpfDigitado = "";

const unsigned long tempoTampaAbertaMs   = 2000;
const unsigned long tempoSemLeituraMaxMs = 45000;
unsigned long momentoUltimaLeitura = 0;

// Função para verificar se o CPF está na lista
bool cpfEstaAutorizado(String cpf) {
  for (int i = 0; i < QTD_CPFS; i++) {
    if (cpf == CPFS_AUTORIZADOS[i]) {
      return true;
    }
  }
  return false;
}

void mostrarTelaCPF() {
  tela.clear();
  tela.setCursor(0, 0);
  tela.print("Digite seu CPF");
  tela.setCursor(0, 1);
  tela.print("CPF: ");
}

void abrirTampa() {
  servoEsquerdo.write(anguloAberto);
  servoDireito.write(anguloFechado);
}

void fecharTampa() {
  servoEsquerdo.write(anguloFechado);
  servoDireito.write(anguloAberto);
}

void processarTecladoCPF() {
  char tecla = teclado.getKey();
  if (tecla == NO_KEY) return;

  if (tecla == '*') {
    cpfDigitado = "";
    tela.setCursor(0, 1);
    tela.print("CPF:           ");
    tela.setCursor(5, 1);
    return;
  }

  if (tecla == '#') {
    if (cpfDigitado.length() != 11) {
      tela.clear();
      tela.setCursor(0, 0);
      tela.print("CPF incompleto");
      delay(1500);
      cpfDigitado = "";
      mostrarTelaCPF();
      return;
    }

    // Verifica se o CPF está na lista de autorizados
    if (cpfEstaAutorizado(cpfDigitado)) {
      tela.clear();
      tela.setCursor(0, 0);
      tela.print("CPF correto :)");
      tela.setCursor(0, 1);
      tela.print("Leitor ativo");
      delay(1500);

      estadoAtual = LEITOR_ATIVO;
      momentoUltimaLeitura = millis();
      tela.clear();
      tela.setCursor(0, 0);
      tela.print("Aproxime o");
      tela.setCursor(0, 1);
      tela.print("medicamento");
    } else {
      tela.clear();
      tela.setCursor(0, 0);
      tela.print("CPF invalido!");
      delay(1500);
      cpfDigitado = "";
      mostrarTelaCPF();
    }
    return;
  }

  if (tecla >= '0' && tecla <= '9') {
    if (cpfDigitado.length() < TAMANHO_MAX_CPF) {
      cpfDigitado += tecla;
      tela.setCursor(5, 1);
      tela.print("           ");
      tela.setCursor(5, 1);
      tela.print(cpfDigitado);
    }
  }
}

void processarLeitor() {
  unsigned long agora = millis();

  char tecla = teclado.getKey();
  if (tecla == 'A') {
    tela.clear();
    tela.setCursor(0, 0);
    tela.print("Teste servo A");
    abrirTampa();
    delay(tempoTampaAbertaMs);
    fecharTampa();
    tela.clear();
    tela.setCursor(0, 0);
    tela.print("Aproxime o");
    tela.setCursor(0, 1);
    tela.print("medicamento");
    momentoUltimaLeitura = agora;
  }

  if (portaLeitor.available() > 0) {
    String codigoLido = portaLeitor.readStringUntil('\n');
    codigoLido.trim();

    if (codigoLido.length() > 0) {
      momentoUltimaLeitura = agora;

      tela.clear();
      tela.setCursor(0, 0);
      tela.print("Recebendo med.");
      abrirTampa();
      delay(tempoTampaAbertaMs);
      fecharTampa();

      tela.clear();
      tela.setCursor(0, 0);
      tela.print("Desconto de 15%");
      tela.setCursor(0, 1);
      tela.print("recebido");
      
      delay(3000);  // Tempo para visualizar a mensagem
      
      tela.clear();
      tela.setCursor(0, 0);
      tela.print("Aproxime o");
      tela.setCursor(0, 1);
      tela.print("medicamento");
    }
  }

  if (agora - momentoUltimaLeitura > tempoSemLeituraMaxMs) {
    estadoAtual = ESPERANDO_CPF;
    cpfDigitado = "";
    fecharTampa();
    mostrarTelaCPF();
  }
}

void setup() {
  tela.init();
  tela.backlight();
  mostrarTelaCPF();

  servoEsquerdo.attach(pinoServoEsquerdo);
  servoDireito.attach(pinoServoDireito);
  fecharTampa();

  portaLeitor.begin(9600);
  portaLeitor.setTimeout(50);
}

void loop() {
  if (estadoAtual == ESPERANDO_CPF) {
    processarTecladoCPF();
  } else if (estadoAtual == LEITOR_ATIVO) {
    processarLeitor();
  }
}