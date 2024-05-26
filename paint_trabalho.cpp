/*
 * Computacao Grafica
 * Codigo Exemplo: Rasterizacao de Segmentos de Reta com GLUT/OpenGL
 * Autor: Prof. Laurindo de Sousa Britto Neto
 */

// Bibliotecas utilizadas pelo OpenGL
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <forward_list>
#include "glut_text.h"
#include <vector>

using namespace std;

// Variaveis Globais
#define ESC 27

//Enumeracao com os tipos de formas geometricas
enum tipo_forma{LIN = 1, TRI = 2, RET = 3, POL = 4, CIR = 5}; // Linha, Triangulo, Retangulo Poligono, Circulo
enum tipo_trans {TRA = 1, SCA = 2, ROT = 3, REF = 4, CIS = 5};

//Verifica se foi realizado o primeiro clique do mouse
bool click1 = false;
bool click2 = false;

//Coordenadas da posicao atual do mouse
int m_x, m_y;

//Coordenadas do primeiro clique, do segundo clique do mouse e do terceiro
int x_1, y_1, x_2, y_2, x_3, y_3;

//tipo forma e transformacao
int modo = LIN, transf = -1;


//Largura e altura da janela
int width = 512, height = 512;

// Definicao de vertice
struct vertice{
    int x;
    int y;
};

// Definicao das formas geometricas
struct forma{
    int tipo;
    forward_list<vertice> v; //lista encadeada de vertices
};

// Lista encadeada de formas geometricas
forward_list<forma> formas;

// Funcao para armazenar uma forma geometrica na lista de formas
// Armazena sempre no inicio da lista
void pushForma(int tipo){
    forma f;
    f.tipo = tipo;
    formas.push_front(f);
}

// Funcao para armazenar um vertice na forma do inicio da lista de formas geometricas
// Armazena sempre no inicio da lista
void pushVertice(int x, int y){
    vertice v;
    v.x = x;
    v.y = y;
    formas.front().v.push_front(v);
}

//Fucao para armazenar uma Linha na lista de formas geometricas
void pushLinha(int x1, int y1, int x2, int y2){
    pushForma(LIN);
    pushVertice(x1, y1);
    pushVertice(x2, y2);
}

void pushRetangulo(int x1, int y1, int x2, int y2){
    pushForma(RET);
    pushVertice(x1, y1);
    pushVertice(x2, y2);
}

void pushTriangulo(int x1, int y1, int x2, int y2, int x3, int y3){
    pushForma(TRI);
    pushVertice(x1, y1);
    pushVertice(x2, y2);
    pushVertice(x3, y3);
}

/*void push_poligono(int x[], int y[]){
    pushForma(POL);
    for (std::vector<vertice>::iterator v = poligs.begin(); v != poligs.end(); ++v)	pushVertice(v->x, v->y);
}*/

void pushCirculo(int x1, int y1, int x2, int y2)
{
	pushForma(CIR);
	pushVertice(x1, y1);
    pushVertice(x2, y2);
}

/*
 * Declaracoes antecipadas (forward) das funcoes (assinaturas das funcoes)
 */
void init(void);
void reshape(int w, int h);
void display(void);
void menu_popup(int value);
void menu_transf(int value);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mousePassiveMotion(int x, int y);
void drawPixel(int x, int y);
// Funcao que percorre a lista de formas geometricas, desenhando-as na tela
void drawFormas();
// Funcao que implementa o Algoritmo Imediato para rasterizacao de segmentos de retas
void retaImediata(double x1,double y1,double x2,double y2);
void Bresenham(double x1, double y1, double x2, double y2);
void desenha_quadrilatero(double x1, double y1, double x2, double y2);
void desenha_triangulo(double x1, double y1, double x2, double y2, double x3, double y3);
void desenha_circulo(double x1, double x2, double raio);
void translacao(int x, int y);
void escala(float sx, float sy);
void rotacao(float angle);
void reflexao(bool horizontal, bool vertical);
void cisalhamento(float shx, float shy);

/*
 * Funcao principal
 */
int main(int argc, char** argv){
    glutInit(&argc, argv); // Passagens de parametro C para o glut
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB); //Selecao do Modo do Display e do Sistema de cor
    glutInitWindowSize (width, height);  // Tamanho da janela do OpenGL
    glutInitWindowPosition (100, 100); //Posicao inicial da janela do OpenGL
    glutCreateWindow ("Computacao Grafica: Paint"); // Da nome para uma janela OpenGL
    
    init(); // Chama funcao init();
    
    glutReshapeFunc(reshape); //funcao callback para redesenhar a tela
    glutKeyboardFunc(keyboard); //funcao callback do teclado
    glutMouseFunc(mouse); //funcao callback do mouse
    glutPassiveMotionFunc(mousePassiveMotion); //fucao callback do movimento passivo do mouse
    glutDisplayFunc(display); //funcao callback de desenho
    
    glutCreateMenu(menu_transf);
    glutAddMenuEntry("Translacao", TRA);
    glutAddMenuEntry("Escala", SCA);
    glutAddMenuEntry("Rotacao", ROT);
    glutAddMenuEntry("Reflexao", REF);
    glutAddMenuEntry("Cisalhamento", CIS);
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
    
    // Define o menu pop-up
    glutCreateMenu(menu_popup);
    glutAddMenuEntry("Linha", LIN);
    glutAddMenuEntry("Retangulo", RET);
    glutAddMenuEntry("Triangulo", TRI);
	glutAddMenuEntry("Poligono", POL);
	glutAddMenuEntry("Circunferencia", CIR);
    glutAddMenuEntry("Sair", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    
    glutMainLoop(); // executa o loop do OpenGL
    return EXIT_SUCCESS; // retorna 0 para o tipo inteiro da funcao main();
}

/*
 * Inicializa alguns parametros do GLUT
 */
void init(void){
    glClearColor(1.0, 1.0, 1.0, 1.0); //Limpa a tela com a cor branca;
}

/*
 * Ajusta a projecao para o redesenho da janela
 */
void reshape(int w, int h)
{
	// Muda para o modo de projecao e reinicializa o sistema de coordenadas
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Definindo o Viewport para o tamanho da janela
	glViewport(0, 0, w, h);
	
	width = w;
	height = h;
    glOrtho (0, w, 0, h, -1 ,1);  

   // muda para o modo de desenho
	glMatrixMode(GL_MODELVIEW);
 	glLoadIdentity();

}

/*
 * Controla os desenhos na tela
 */
void display(void){
    glClear(GL_COLOR_BUFFER_BIT); //Limpa o buffer de cores e reinicia a matriz
    glColor3f (0.0, 0.0, 0.0); // Seleciona a cor default como preto
    drawFormas(); // Desenha as formas geometricas da lista
    //Desenha texto com as coordenadas da posicao do mouse
    draw_text_stroke(0, 0, "(" + to_string(m_x) + "," + to_string(m_y) + ")", 0.2);
    glutSwapBuffers(); // manda o OpenGl renderizar as primitivas

}

/*
 * Controla o menu pop-up
 */
void menu_popup(int value){
    if (value == 0) exit(EXIT_SUCCESS);
    modo = value;
}

void menu_transf(int value)
{
	if (value == 0) exit(EXIT_SUCCESS);
	switch (value){
		case 1: translacao(30, 30); break;
		/*case 2: escala(0.4, 0.4); break;
		case 3: rotacao(45); break;
		case 4: reflexao(false, true); break;
		case 5: cisalhamento(0.5, 0); break;*/
	}
	
	modo = value;
}

/*
 * Controle das teclas comuns do teclado
 */
void keyboard(unsigned char key, int x, int y){
    switch (key) { // key - variavel que possui valor ASCII da tecla precionada
        case ESC: exit(EXIT_SUCCESS); break;
    }
}

/*
 * Controle dos botoes do mouse
 */
void mouse(int button, int state, int x, int y){
    switch (button) {
        case GLUT_LEFT_BUTTON:
            switch(modo){
                case LIN:
                    if (state == GLUT_DOWN) {
                        if(click1){
                            x_2 = x;
                            y_2 = height - y - 1;
                            pushLinha(x_1, y_1, x_2, y_2);
                            click1 = false;
                            glutPostRedisplay();
                        }else{
                            click1 = true;
                            x_1 = x;
                            y_1 = height - y - 1;
                        }
                    }
                	break;
            
            	case RET:
                    if (state == GLUT_DOWN) {
                        if(click1){
                            x_2 = x;
                            y_2 = height - y - 1;
                            pushRetangulo(x_1, y_1, x_2, y_2);
                            click1 = false;
                            glutPostRedisplay();
                        }else{
                            click1 = true;
                            x_1 = x;
                            y_1 = height - y - 1;
                        }
                    }
                	break;
                
                case TRI:
                    if (state == GLUT_DOWN) {
                        if(click1){
                            x_3 = x;
                            y_3 = height - y - 1;
                            click1 = false;
                            click2 = true;
                        }
						else if (click2)
						{
							x_2 = x;
                            y_2 = height - y - 1;
                            pushTriangulo(x_1, y_1, x_2, y_2, x_3, y_3);
                            click2 = false;
                            glutPostRedisplay();
                        }
                        else
						{
							click1 = true;
							x_1 = x;
							y_1 = height - y - 1;
						}
                    }
                	break;
                case CIR:
                    if (state == GLUT_DOWN) {
                        if (click1)
						{
							click1 = false;
							x_2 = x;
							y_2 = height - y - 1;
							pushCirculo(x_1, y_1, x_2, y_2);
							glutPostRedisplay();
						}
						else
						{
							click1 = true;
							x_1 = x;
							y_1 = height - y - 1;
						}
       	   	   	   }
                	break;
            }
        break;

//        case GLUT_MIDDLE_BUTTON:
//            if (state == GLUT_DOWN) {
//                glutPostRedisplay();
//            }
//        break;
//
//        case GLUT_RIGHT_BUTTON:
//            if (state == GLUT_DOWN) {
//                glutPostRedisplay();
//            }
//        break;
            
    }
}

/*
 * Controle da posicao do cursor do mouse
 */
void mousePassiveMotion(int x, int y){
    m_x = x; m_y = height - y - 1;
    glutPostRedisplay();
}

/*
 * Funcao para desenhar apenas um pixel na tela
 */
void drawPixel(int x, int y){
    glBegin(GL_POINTS); // Seleciona a primitiva GL_POINTS para desenhar
        glVertex2i(x, y);
    glEnd();  // indica o fim do ponto
}

/*
 *Funcao que desenha a lista de formas geometricas
 */

void drawFormas()
{
    if(modo == LIN) 
	{
		if (click1) retaImediata(x_1, y_1, m_x, m_y);
	}
    
    if (modo == RET)
	{
		if(click1) desenha_quadrilatero(x_1, y_1, m_x, m_y);
	} 

	if (modo == TRI)
	{
		if (click1) retaImediata(x_1, y_1, m_x, m_y);
		if(click2) desenha_triangulo(x_1, y_1, m_x, m_y, x_3, y_3);
	}
    
	if (modo == CIR)
	{
		double raio = sqrt(pow(x_1 - m_x, 2) + pow(y_1 - m_y, 2));
    	if(click1) desenha_circulo(x_1, y_1, raio);
	}

	if (modo == POL)
	{
		if (click1) retaImediata(x_1, y_1, m_x, m_y);
	}
    
    //Percorre a lista de formas geometricas para desenhar
    for(forward_list<forma>::iterator f = formas.begin(); f != formas.end(); f++){
    	int i = 0;
		vector<double> x, y;
        switch (f->tipo) {
            case LIN:
                //Percorre a lista de vertices da forma linha para desenhar
                for(forward_list<vertice>::iterator v = f->v.begin(); v != f->v.end(); v++, i++){
                    x.push_back(v->x);
                    y.push_back(v->y);
                }
                
                //Desenha o segmento de reta apos dois cliques
                Bresenham(x[0], y[0], x[1], y[1]);
            	break;
            
            case RET:
                for(forward_list<vertice>::iterator v = f->v.begin(); v != f->v.end(); v++, i++){
                    x.push_back(v->x);
                    y.push_back(v->y);
                }
                
            	// Desenha quadrilatero
                desenha_quadrilatero(x[0], y[0], x[1], y[1]);
            	break;
        
        	case TRI:
        		for(forward_list<vertice>::iterator v = f->v.begin(); v != f->v.end(); v++, i++){
                    x.push_back(v->x);
                    y.push_back(v->y);
                }
                
            	// Desenha triangulo
                desenha_triangulo(x[0], y[0], x[1], y[1], x[2], y[2]);
            	
            	break;
            
   			/*case POL:
        		for(forward_list<vertice>::iterator v = f->v.begin(); v != f->v.end(); v++, i++){
                    x[i] = v->x;
                    y[i] = v->y;
                }
                
            	// Desenha poligono
                desenha_poligono(x, y);
            	
            	break;*/
            
            case CIR:
        		for(forward_list<vertice>::iterator v = f->v.begin(); v != f->v.end(); v++, i++){
                    x.push_back(v->x);
                    y.push_back(v->y);
                }
                
            	// Desenha circulo
            	double raio = sqrt(pow(x[0] - x[1], 2) + pow(y[0] - y[1], 2));
                desenha_circulo(x[1], y[1], raio);
            	
            	break;
        }
    }
}

void retaImediata(double x1, double y1, double x2, double y2){
    double m, b, yd, xd;
    double xmin, xmax,ymin,ymax;
    
    drawPixel((int)x1,(int)y1);
    if(x2-x1 != 0){ //Evita a divisao por zero
        m = (y2-y1)/(x2-x1);
        b = y1 - (m*x1);

        if(m>=-1 && m <= 1){ // Verifica se o declive da reta tem tg de -1 a 1, se verdadeira calcula incrementando x
            xmin = (x1 < x2)? x1 : x2;
            xmax = (x1 > x2)? x1 : x2;

            for(int x = (int)xmin+1; x < xmax; x++){
                yd = (m*x)+b;
                yd = floor(0.5+yd);
                drawPixel(x,(int)yd);
            }
        }else{ // Se tg menor que -1 ou maior que 1, calcula incrementado os valores de y
            ymin = (y1 < y2)? y1 : y2;
            ymax = (y1 > y2)? y1 : y2;

            for(int y = (int)ymin + 1; y < ymax; y++){
                xd = (y - b)/m;
                xd = floor(0.5+xd);
                drawPixel((int)xd,y);
            }
        }

    }else{ // se x2-x1 == 0, reta perpendicular ao eixo x
        ymin = (y1 < y2)? y1 : y2;
        ymax = (y1 > y2)? y1 : y2;
        for(int y = (int)ymin + 1; y < ymax; y++){
            drawPixel((int)x1,y);
        }
    }
    drawPixel((int)x2,(int)y2);
}

// ok!!!
void Bresenham(double x1, double y1, double x2, double y2)
{
	double incE, incNE, d, temp, deltax, deltay, xi, yi;
	bool D = false, S = false;
	
	deltax = x2 - x1;
	deltay = y2 - y1;
	
	// reduzindo ao primeiro octante
	if (deltax * deltay < 0)
	{
		 S = true;
		 y1 = -y1;
		 y2 = -y2;
		 deltax = x2 - x1;
		 deltay = y2 - y1;
	}
	
	if (abs(deltax) < abs(deltay))
	{
		D = true;
		double temp = x1;
		x1 = y1;
		y1 = temp;
		
		temp = x2;
		x2 = y2;
		y2 = temp;
		
		deltax = x2 - x1;
        deltay = y2 - y1;
	}
	
	if (x1 > x2)
	{
		double temp = x1;
		x1 = x2;
		x2 = temp;
		
		temp = y1;
		y1 = y2;
		y2 = temp;
		deltax = x2 - x1;
		deltay = y2 - y1;
	}
	
	incNE = 2 * (deltay - deltax);
	incE = 2 * deltay;
	d = 2 * deltay - deltax;
	xi = x1, yi = y1;
	
	drawPixel(xi, yi);
	
	while(xi < x2)
	{
		if (d <= 0) d += incE;
		else
		{
			d += incNE;
			yi++;
		}
		
		xi++;
		
		if(S && D) drawPixel(yi, -xi);
        else if(S) drawPixel(xi, -yi);
        else if(D) drawPixel(yi, xi);
        else drawPixel(xi, yi);
	}
	
	drawPixel(xi, yi);
}

void desenha_quadrilatero(double x1, double y1, double x2, double y2)
{
	Bresenham(x1, y1, x1, y2);
	Bresenham(x1, y2, x2, y2);
	Bresenham(x2, y2, x2, y1);
	Bresenham(x2, y1, x1, y1);
}

void desenha_triangulo(double x1, double y1, double x2, double y2, double x3, double y3)
{
	Bresenham(x1, y1, x2, y2);
	Bresenham(x2, y2, x3, y3);
	Bresenham(x3, y3, x1, y1);
}

/*void desenha_poligono(vector<int> &poligono)
{
	for (int i = 0; i < poligono.size() -1; i++) retaBresenham(x[i], y[i], x[i + 1], y[i + 1]);
}*/

void desenha_circulo(double x1, double y1, double raio)
{
	int d = 1 - raio, incE = 3, incSE = (-2) * raio + 5;

    drawPixel(x1, y1 + raio);
    drawPixel(x1 + raio, y1);
    drawPixel(x1, y1 + raio);
    drawPixel(x1 - raio, y1);

    int xi = 0, yi = raio;

	while(yi > xi)
    {
        if (d < 0) 
		{
            d += incE;
            incE += 2;
            incSE += 2;
        } 
		else 
		{
            d += incSE;
            incE += 2;
            incSE += 4;
            yi--;
        }
        
        xi++;

		drawPixel(x1 - yi, y1 - xi);
		drawPixel(x1 - yi, y1 + xi);
        drawPixel(x1 + xi, y1 + yi);
        drawPixel(x1 + yi, y1 - xi);
        drawPixel(x1 + yi, y1 + xi);
        drawPixel(x1 - xi, y1 + yi);
        drawPixel(x1 + xi, y1 - yi);
        drawPixel(x1 - xi, y1 - yi);
    }
}

void translacao(int x, int y) 
{
    for (forward_list<forma>::iterator it_forma = formas.begin(); it_forma != formas.end(); ++it_forma) 
	{
        for (forward_list<vertice>::iterator it_vertice = it_forma->v.begin(); it_vertice != it_forma->v.end(); ++it_vertice) 
		{
            it_vertice->x += x;
            it_vertice->y += y;
        }
    }
    glutPostRedisplay();
}
