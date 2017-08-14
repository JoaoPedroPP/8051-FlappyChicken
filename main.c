#include <8051.h>
#include <stdlib.h> //Para utilizar a fun��o rand()

#define DISPLAY_IR	0xFFC2 //Endere�o do display LCD
#define DISPLAY_DR	0xFFD2 //Endere�o do display LCD
#define DISPLAY_BF	0xFFE2 //Endere�o do display LCD
#define DISPLAY_R	0xFFF2 //Endere�o do display LCD
#define DAC			0xFFE4 //Endere�o do DAC

static unsigned char far at DISPLAY_IR LCD_cmd; //Mapeamento em mem�ria do display LCD (enviar comandos)
static unsigned char far at DISPLAY_DR LCD_data; //Mapeamento em mem�ria do display LCD (enviar dados)
unsigned char far at DAC amost; //Mapeamento em mem�ria do DAC

code unsigned char valores[] = {128,128};
unsigned int length = 2; //N�mero de pontos na m�sica
unsigned int pos; //Posi��o atual no vetor de amplitudes da m�sica

code unsigned char pessoa[] = {0x00, 0x04, 0x04, 0x1E, 0x04, 0x04, 0x00, 0x00}; //Caracter customizado - pessoa
code unsigned char cacto[] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}; //Caracter customizado - cacto
code unsigned char missel[] = {0x00, 0x01, 0x0F, 0x1F, 0x0F, 0x01, 0x00, 0x00}; //Caracter customizado - m�ssel
unsigned char rand_pipe[16];
volatile unsigned char x_pos = 3; //Posi��o do personagem na horizontal
volatile unsigned char y_pos = 1; //Posi��o do personagem na horizontal
unsigned char jump = 1; //jump = 0 no ch�o, jump > 0 pulando
unsigned char cactos[16]; //Armazena a posi��o dos cactos
unsigned char missels[16]; //Armazena a posi��o dos m�sseis
unsigned char spawn; //Cria um espa�o minimo entre os obst�culos
unsigned char points; //Armazena os pontos do jogador
unsigned char lost; //lost = 0 n�o perdeu, lost > 0 perdeu
unsigned int t0ms = 0;
unsigned int fall_interval = 1000;
unsigned int fall_bird = 1000;

void delay(unsigned int interval) //Rotina de atraso
{
	unsigned int i;
	while (interval-- > 0)
	{
		for (i = 0; i < 127; i++)
		{
		
		}
	}
}

void sendCmd(unsigned char cmd) //Rotina para enviar comandos para o display LCD
{
	LCD_cmd = cmd;
	delay(1);
}

void sendChar(unsigned char c) //Rotina para enviar dados para o display LCD
{
	LCD_data = c;
	delay(1);
}

void sendString(unsigned char string[]) //Rotina para enviar uma string para o display LCD
{
	unsigned char i = 0;
	while (string[i] != 0x00) //Envia caractere por caractere at� encontrar 0x00 (fim da string)
	{
		sendChar(string[i]);
		i++;
	}
}

void gotoPos(unsigned char row, unsigned char col) //Rotina para mover o cursor do display LCD
{
	switch (row)
	{
		case 0:
			sendCmd(0x80 + col);
			break;
		case 1:
			sendCmd(0xC0 + col);
			break;
		case 2:
			sendCmd(0x90 + col);
			break;
		case 3:
			sendCmd(0xD0 + col);
			break;
	}
}

void clearScreen() //Rotina para limpar a tela do display LCD
{
	sendCmd(0x01);
	delay(9);
}

void timer_interrupt() interrupt 1 //Rotina de interrup��o do TIMER0
{
	t0ms++;
	TH0 = 0xFC;
	TL0 = 0x66;
}

void drawChar(unsigned char new_jump, unsigned char new_x) //Rotina para desenhar o personagem
{
	//gotoPos(3 - (jump > 0), x_pos); //Move o cursor para onde o personagem est� posicionado
	//sendChar(' '); //Envia o caractere espa�o
	//x_pos = new_x; //Altera a posi��o horizontal do personagem para o novo valor
	//y_pos = new_jump; //Altera se o personagem est� pulando para o novo valor
	gotoPos(y_pos, x_pos); //Move o cursor para a nova posi��o do personagem
	sendChar(0x00); //Envia o caractere customizado da pessoa
	//if (cactos[x_pos] == 1 && jump == 0 || missels[x_pos] == 1 && jump > 0) //Testa colis�o com os obst�culos
	//{
	//	lost = 1; //Se houve colis�o perdeu
	//}
}

void drawObj(unsigned char index,unsigned char y, unsigned char obj, unsigned char flag) //Rotina para desenhar o obst�culo
{
	gotoPos(y, index + 1); //Move o cursor para posi��o do obst�culo
	sendChar(' '); //Envia o caractere espa�o
	gotoPos(y, index); //Move o cursor para a nova posi��o do obst�culo
	if(y == flag) {
		//sendChar(' ');
	}
	else{
		sendChar(obj); //Envia o caractere customizado do obst�culo (cacto ou m�ssel)
	}
}

void drawPoints() //Rotina para escrever a pontua��o e o n�vel atual
{
	gotoPos(0, 8); //Move o cursor para a linha 0 coluna 8
	sendChar(points / 10 + 0x30); //Escreve a dezena da pontua��o
	sendChar(points % 10 + 0x30); //Escreve a unidade da pontua��o
	gotoPos(1, 7); //Move o cursor para a linha 1 coluna 7
	sendChar(points / 5 + 0x30); //Envia o valor do n�vel
}

void serial_interrupt() interrupt 4 //Rotina de interrup��o da serial
{
	if (lost == 0) //Se n�o perdeu
	{
		switch (SBUF)
		{/*
			case 0x34: //Caso seja pressionado 4
				if (x_pos > 0) //Se o personagem n�o chegou no limite a esquerda
				{
					drawChar(jump, x_pos - 1); //Move o personagem uma posi��o para esquerda
				}
				break;
			case 0x36: //Caso seja pressionado 6
				if (x_pos < 15) //Se o personagem n�o chegou no limite a direita
				{
					drawChar(jump, x_pos + 1); //Move o personagem uma posi��o para direita
				}
				break;*/
			case 0x38: //Caso seja pressionado 8
				if (y_pos > 0) //Se o personagem n�o estiver pulando
				{	gotoPos(y_pos, x_pos);
					t0ms = 0;
					sendChar(' ');
					drawChar(y_pos--, x_pos); //O personagem pula
				}
				break;
		}
	}
	else if (lost == 2 && SBUF == ' ') //Se perdeu e recebeu a tecla espa�o reinicia o jogo
	{
		lost = 0;
	}
	RI = 0; //Limpa a flag de recep��o via serial
}

void main()
{
	unsigned char i; //Vari�vel para os loops
	unsigned int r; //Vari�vel para armazenar o valor aleat�rio gerado
	IE = 0x92; //Ativa as interrup��es da serial e do TIMER0 (EA = 1, ES = 1 e ET0 = 1)
	TMOD = 0x21; //Configura o TIMER0 no modo 1 e o TIMER1 no modo 2
	TH0 = 0xFC; //Configura o TIMER0 para frequ�ncia de 2kHz (taxa de amostragem da m�sica)
	TL0 = 0x66; //Configura o TIMER0 para frequ�ncia de 2kHz (taxa de amostragem da m�sica)
	TR0 = 1; //Liga o TIMER0
	TF0 = 0;
	TH1 = 0xFD; //Configura o TIMER1 para gerar baud rate de 9600
	TL1 = 0xFD; //Configura o TIMER1 para gerar baud rate de 9600
	TR1 = 1; //Liga o TIMER1
	TF1 = 0;
	SCON = 0x50; //Configura a serial no modo 1 e ativa a recep��o de dados
	sendCmd(0x38); //Inicia o display com 8 bits e 5x7
	sendCmd(0x0C); //Liga o display e desliga o cursor
	sendCmd(0x40); //Escreve na CGRAM
	for (i = 0; i < 8; i++) //Envia o caractere customizado - pessoa
	{
		sendChar(pessoa[i]);
	}
	for (i = 0; i < 8; i++) //Envia o caractere customizado - cacto
	{
		sendChar(cacto[i]);
	}
	for (i = 0; i < 8; i++) //Envia o caractere customizado - m�ssel
	{
		sendChar(missel[i]);
	}
	while (1)
	{
		gotoPos(0, 0); //Move o cursor para linha 0 coluna 0
		//sendString("Pontos: 00"); //Escreve a string de pontos
		gotoPos(1, 0); //Move o cursor para linha 1 coluna 0
		//sendString("Nivel: 0"); //Escreve a string de n�vel
		gotoPos(1, 3); //Move o cursor para linha 3 coluna 0
		sendChar(0x00); //Escreve o caractere do personagem
		while (lost == 0) //Enquanto n�o perder
		{/*
			if (jump > 0) //Se o pulo for maior que 0
			{
				jump++; //Incrementa o valor de pulo
			}
			if (jump >= 3) //Se o pulo for maior ou igual a 3
			{
				drawChar(0, x_pos); //Faz o personagem cair
			}*/
			if (t0ms > 100 && y_pos < 3){
				t0ms = 0;
				gotoPos(y_pos, x_pos);
				//sendChar(' ');
				//drawChar(y_pos++, x_pos); //Faz o personagem cair
			}
			if (cactos[2] == 1) //Se existe um cacto no in�cio da tela
			{
				cactos[2] = 0; //Remove o cacto
				gotoPos(3, 2); //Move o cursor para linha 3 coluna 0
				sendChar(' '); //Envia o caractere espa�o
				gotoPos(2, 2);
				sendChar(' ');
				gotoPos(1, 2);
				sendChar(' ');
				gotoPos(0, 2);
				sendChar(' ');
				//points++; //Incrementa a pontua��o
				//drawPoints(); //Escreve a pontua��o
			}
			//rand_pipe[0] = 2;
			for (i = 0; i < 15; i++) //Loop para mover os cactos
			{
				if (cactos[i] != cactos[i + 1]) //Se n�o houver cacto em i e houver em i+1
				{	r = rand();
					if(r>= 0 && r < 8191){
						rand_pipe[i] = 0;
					}
					if(r >= 8191 && r < 16382){
						rand_pipe[i] = 1;
					}
					if(r >= 16382 && r < 24573){
						rand_pipe[i] = 2;
					}
					if(r >= 24573 && r < 32764){
						rand_pipe[i] = 3;
					}
					drawObj(i,0, 0x01, rand_pipe[i-1]); //Desenha o cacto na posi��o nova
					drawObj(i,1, 0x01, rand_pipe[i-1]);
					drawObj(i,2, 0x01, rand_pipe[i-1]);
					drawObj(i,3, 0x01, rand_pipe[i-1]);
					gotoPos(y_pos, x_pos);
					sendChar(0x00);
				}
				cactos[i] = cactos[i + 1]; //Move o cacto uma posi��o para esquerda
				rand_pipe[i-1] = rand_pipe[i];
				cactos[i + 1] = 0; //Remove o cacto da posi��o antiga
//				rand_pipe[i + 1] = 0;
			}
			if (missels[0] == 1) //Se existe um m�ssel no in�cio da tela
			{
				missels[0] = 0; //Remove o m�ssel
				gotoPos(2, 0); //Move o cursor para linha 3 coluna 0
				sendChar(' '); //Envia o caractere espa�o
				//points++; //Incrementa a pontua��o
				//drawPoints(); //Escreve a pontua��o
			}/*
			for (i = 0; i < 15; i++) //Loop para mover os m�sseis
			{
				if (missels[i] != missels[i + 1]) //Se n�o houver m�ssel em i e houver em i+1
				{
					drawObj(i, 0x02); //Desenha o m�ssel na posi��o nova
				}
				missels[i] = missels[i + 1]; //Move o m�ssel uma posi��o para esquerda
				missels[i + 1] = 0; //Remove o m�ssel da posi��o antiga
			}*/
			if (spawn < 4) //Se spawn for menor que 4 incrementa o seu valor
			{
				spawn++;
			}
			if (spawn == 4) //Se spawn atingiu o valor 4
			{
				r = rand(); //Gera um n�mero pseudo-aleat�rio entre 0 e 32767
				if (r > 27767) //Se o n�mero for maior que 27767 (5000 possibilidades)
				{
					cactos[15] = 1; //Gera um cacto no final da tela
					spawn = 0; //Reinicia o valor de spawn
				}
				else if (r > 22767) //Sen�o se o n�mero for maior que 22767 (5000 possibilidades)
				{
					missels[15] = 1; //Gera um cacto no final da tela
					spawn = 0; //Reinicia o valor de spawn
				}
			}
//			if (cactos[x_pos] == 1 && jump == 0 || missels[x_pos] == 1 && jump > 0) //Testa colis�o com os obst�culos
			if (y_pos != rand_pipe[1] && cactos[x_pos])
			{
				lost = 1; //Se houve colis�o perdeu
			}
			delay(500 - (points / 5 * 50)); //Atraso que se torna menor conforme avan�a de n�vel
		}
		lost = 2; //Indica que est� na tela de fim de jogo
		clearScreen(); //Limpa a tela do display LCD
		gotoPos(1, 0); //Move o cursor para a linha 1 coluna 0
		sendString("_|_ FUCK YOU _|_"); //Escreve a string de fim de jogo
		gotoPos(2, 0); //Move o cursor para a linha 2 coluna 0
		//sendChar(points / 10 + 0x30); //Escreve a dezena da pontua��o
		//sendChar(points % 10 + 0x30); //Escreve a unidade da pontua��o
		//sendString(" pontos"); //Escreve a string de pontos
		while (lost == 2); //Trava o programa enquanto lost for igual a 2
		clearScreen(); //Limpa a tela
		for (i = 0; i < 16; i++) //Reinicia os vetores dos obst�culos
		{
			cactos[i] = 0;
			missels[i] = 0;
		}
		x_pos = 3; //Reinicia a posi��o do personagem
		y_pos = 1;
		jump = 0; //Reinicia o valor de jump
		points = 0; //Reinicia a pontua��o
	}
}
