#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#define botao 2
#define buzzer 3

LiquidCrystal_I2C lcd(0x27, 16, 2);

int vida;
int telaAtual;
int recorde = 0;
double pontos;
double velocidade = 150;
bool jogo = false;
int cooldown = 0;
int imune = 0;
int carinhaY; // posição y em que o carinha esta (baixo ou cima)

int obs[26];
int cont = 0;

int push1 = 0;
int push2 = 0;
int aux = 0;
int cadeia[13][6] = 
{
    0,0,1,1,1,0,0,0,0,1,1,0,0,
    0,0,1,0,0,0,0,1,1,1,1,0,0,
    0,0,1,0,0,0,1,0,0,0,1,0,0,
    0,0,1,1,1,1,0,0,0,1,1,0,0,
    0,0,0,1,1,0,0,0,1,1,0,0,0,
    0,0,1,1,1,0,0,0,1,1,1,0,0,
};

// caracteres customizados
byte carinha[] = {B01111, B01011, B01110, B01111, B00110, B10111, B11110, B00101};
byte obstaculo[] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};
byte coracao[] = {B00000, B00000, B01010, B11111, B11111, B01110, B00100, B00000};
byte vazio[] = {B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00000};

void setup()
{
  /*
  Setando a memoria, você guarda 1 espaço pra checar se pode usar o proximo
  e o proximo vc guarda toda a informação necesssária
  */
  if (EEPROM[0] != 1)
  {
    EEPROM[0] = 1;
    EEPROM[1] = recorde;
  }
  else
  {
    recorde = EEPROM[1];
  }

  lcd.init();
  lcd.backlight();
  lcd.createChar(1, carinha);
  lcd.createChar(2, obstaculo);
  lcd.createChar(3, coracao);
  lcd.createChar(4, vazio);

  pinMode(botao, INPUT_PULLUP);

  carinhaY = 1; // começa no chão
}

void loop()
{

  if (jogo)
  {

    painel(vida);
    pontos += 0.1;
    velocidade -= 0.1;

    /*caso tenha ocorrido uma colisao anterior, o carinha fica
    imune por determinado periodo, a fim de evitar a perda
    sequencial das 3 vidas*/

    if (imune > 0 && imune < 4)
      imune++; // se esta imune, consome imunidade
    if (imune >= 4)
      imune = 0; // retorna ao estado nao-imune

    int press = digitalRead(botao);

    if (press && cooldown < 3)
    {
      carinhaY = 0; // pula
      cooldown++;
      tone(buzzer, 440);
    }
    else
    {
      // se ta pulando e nao tem obstaculo embaixo, volta pro chao
      noTone(buzzer);
      if (carinhaY == 0 and obs[0] == 0)
        carinhaY = 1;
      if (carinhaY == 1 and (obs[0] == 1 or obs[1] == 1))
      { // caso haja colisao
        // se nao-imune, perde vida e se torna imune
        if (imune == 0)
        {
          vida--;
          imune++;
        }
      }
      cooldown = 0;
    }

    desenhaCarinha(0, carinhaY);

    if (!vida)
    {                // morre
      telaAtual = 1; // tela de derrota
      recorde = max(recorde, pontos);
      // Setando o recorde na memoria
      EEPROM[1] = recorde;
      jogo = false;
      push1 = push2 = aux = 0;
    }

    // desenha os 13 primeiros obstaculos
    for (int i = 0; i < 13; i++)
    {
      if (i != 0 or (i == 0 and carinhaY == 0))
      {
        if (obs[i])
          desenhaObstaculo(i, 1);
        else
          desenhaVazio(i, 1);
      }
    }

    // desliza toda a cadeia para a esquerda
    for (int i = 0; i < 25; i++)
    {
      obs[i] = obs[i + 1];
    }

    // atualiza a cadeia
    if (cont == 13)
    {
      int r = random(0, 5);
      for (int i = 0; i < 13; i++)
        obs[i + 13] = cadeia[r][i];
      cont = -1;
    }
    else
      obs[25] = 0;
    cont++;

    delay((int)velocidade);
  }
  else
  {
    Tela(telaAtual);
    push1 = digitalRead(botao);
    if (push1 and push2)
      aux++;
    else
    {
      aux = 0;
      push2 = 0;
    }
    push2 = push1;
    // jogo inicia apenas apos pressionar o botao
    if (aux >= 10)
      reset();
  }
}

// retorna o jogo ao estado inicial
void reset()
{
  lcd.clear();
  jogo = true;
  vida = 3;
  pontos = 0;
  velocidade = 150;
  for (int i = 0; i < 26; i++)
    obs[i] = 0;
}

void painel(int v)
{ // parametro - numero de vidas
  for (int i = 15; i >= 13; i--)
  {
    if (16 - i <= v)
      desenhaVida(i, 0);
    else
      desenhaVazio(i, 0);
  }
  lcd.setCursor(13, 1);
  lcd.print((int)pontos);
}

void desenhaVazio(int x, int y)
{
  lcd.setCursor(x, y);
  lcd.write(4);
}

void desenhaCarinha(int x, int y)
{
  lcd.setCursor(x, y);
  lcd.write(1);
  if (y == 1)
    desenhaVazio(x, 0);
  if (y == 0)
    desenhaVazio(x, 1);
}

void desenhaObstaculo(int x, int y)
{
  lcd.setCursor(x, y);
  lcd.write(2);
}

void desenhaVida(int x, int y)
{
  lcd.setCursor(x, y);
  lcd.write(3);
}

void Tela(int k)
{ // 0-tela inicial 1-derrota
  if (k == 0)
  {
    lcd.setCursor(2, 0);
    lcd.print("DINO - SALTO");
    lcd.setCursor(0, 1);
    lcd.print("Pressione por 2s");
  }
  if (k == 1)
  {
    desenhaVazio(0, 0);
    desenhaVazio(15, 0);
    lcd.setCursor(1, 0);
    lcd.print("pts:");
    lcd.print((int)pontos);
    lcd.setCursor(7, 0);
    lcd.print(" max:");
    lcd.print(recorde);
    lcd.setCursor(0, 1);
    lcd.print("Pressione por 2s");
  }
}