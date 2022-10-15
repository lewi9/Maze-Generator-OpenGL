#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <fstream>
#include <iostream>

#include "GL\glew.h"
#include "GL\freeglut.h"

#include "shaderLoader.h" //narzŕdzie do │adowania i kompilowania shaderˇw z pliku

//funkcje algebry liniowej
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective


//#define NAZWA_LABIRYNTU "maze1.txt"
#define NAZWA_LABIRYNTU "maze3.txt"

//Wymiary okna
int screen_width = 640;
int screen_height = 480;

int pozycjaMyszyX; // na ekranie
int pozycjaMyszyY;
int mbutton; // wcisiety klawisz myszy

double kameraX= 180.0;
double kameraZ = 40.0;
double kameraD = -500;
double kameraPredkosc;
double kameraKat = -20;
double kameraPredkoscObrotu;
double poprzednie_kameraX;
double poprzednie_kameraZ;
double poprzednie_kameraD;

double rotation = 0;

//macierze

glm::mat4 MV; //modelview - macierz modelu i świata
glm::mat4 P;  //projection - macierz projekcji, czyli naszej perspektywy
glm::vec3 lightPos(200, 400.0f, 200.0f);
GLuint objectColor_id = 0;
GLuint lightColor_id = 0;
GLuint lightPos_id = 0;
GLuint viewPos_id = 0;
GLuint materialshininess_id = 0;
GLuint materialambient_id = 0;
GLuint materialdiffuse_id = 0;
GLuint materialspecular_id = 0;

//Mapa labiryntu
int** own;

//Obiekt świetlny
float *vertices;
float *normals;
GLuint *elements;

//Blok
float f_vertices[] = {
	0,0,0,
	0,0,-1,
	1,0,-1,
	1,0,0,

	0,-1,0,
	0,-1,-1,
	1,-1,-1,
	1,-1,0,

	0,-1,0,
	0,0,0,
	1,0,0,
	1,-1,0,

	1,-1,0,
	1,0,0,
	1,0,-1,
	1,-1,-1,

	1,-1,-1,
	0,-1,-1,
	0,0,-1,
	1,0,-1,

	0,-1,-1,
	0,-1,0,
	0,0,0,
	0,0,-1,
};

GLuint f_elements[] = {
	0,1,2,3,
	4,5,6,7,
	8,9,10,11,
	12,13,14,15,
	16,17,18,19,
	20,21,22,23
};

float f_normals[] = {
	0,1,0,
	0,1,0,
	0,1,0,
	0,1,0,

	0,-1,0,
	0,-1,0,
	0,-1,0,
	0,-1,0,

	0,0,1,
	0,0,1,
	0,0,1,
	0,0,1,

	1,0,0,
	1,0,0,
	1,0,0,
	1,0,0,
	
	0,0,-1,
	0,0,-1,
	0,0,-1,
	0,0,-1,
	
	-1,0,0,
	-1,0,0,
	-1,0,0,
	-1,0,0,
};

//shaders
GLuint programID = 0;
GLuint lamp_ID = 0;

//Blok
unsigned int FloorVao, FloorVbo, FloorEbo, FloorNormal;

//Obiekt świetlny
unsigned int lightVbo, lightEbo;
unsigned int lightVao;

int n_v, n_el;

//Zmienne do obslugi "gry"
double f_x;
double f_z;
float dx = -0.5;
float dz = 0.6;
float dy = -0.5;
double mnoznikx = 0;
double mnoznikz = 0;
int direction = 0;
int ix = 0;
int iz = 0;
bool play = 0;

double lr = 1;
double lg = 1;
double lb = 1;

void mysz(int button, int state, int x, int y)
{
	mbutton = button;
	switch (state)
	{
	case GLUT_UP:
		break;
	case GLUT_DOWN:
		pozycjaMyszyX = x;
		pozycjaMyszyY = y;
		poprzednie_kameraX = kameraX;
		poprzednie_kameraZ = kameraZ;
		poprzednie_kameraD = kameraD;
		break;
	}
}
/*******************************************/
void mysz_ruch(int x, int y)
{
	if (mbutton == GLUT_LEFT_BUTTON)
	{
		kameraX = poprzednie_kameraX - (pozycjaMyszyX - x) * 0.1;
		kameraZ = poprzednie_kameraZ - (pozycjaMyszyY - y) * 0.1;
	}
	if (mbutton == GLUT_RIGHT_BUTTON)
	{
		kameraD = poprzednie_kameraD + (pozycjaMyszyY - y) * 0.1;
	}
}
/******************************************/


void klawisz(GLubyte key, int x, int y)
{
	switch (key) {

	case 27:    /* Esc - koniec */
		exit(1);
		break;
	case 'p': //Rozpoczecie gry
		if( play ) return;
		play = 1;
		kameraD = 0;
		dz = 0.9;
		if( f_z > 7 ) dz += 0.04;
		if( f_z > 9 ) dz += 0.01;
		if( f_z > 13 ) dz += 0.015;
		if( f_z > 18 ) dz += 0.015;
		if( f_z > 45 ) dz += 0.012;
		if( f_z > 100 ) dz += 0.008;
		dy += sqrt(f_x*f_z)/100 - sqrt( f_x * f_z )/30 * 0.1 - sqrt(f_x*f_z)/(30/(sqrt( f_x * f_z )/40))*0.1;
		ix = f_x / 2;
		iz = f_z - 1;
		if( int( f_x ) % 2 == 0 ) { dx -= 0.5 * mnoznikx; ix -= 1; }
		for( int i = ix; i < f_x; i++ )
		{
			if( !own[iz][i] ) return;
			ix += 1;
			dx += mnoznikx;
		}
		ix--;
		dx -= mnoznikx;
		for( int i = ix; i > -1; i-- )
		{
			if( !own[iz][i] ) return;
			ix -= 1;
			dx -= mnoznikx;
		}

		break;
	case 'q': //Wyjscie z gry widok z gory
		play = 0;
		kameraD = -500;
		dz = 0.6;
		dy = -0.5;
		dx = -0.5;
		break;
	case '1': // Przesuniecie punktu swietlengo w gore
		lightPos[1] += 10;
		break;
	case '2': // Przesuniecie punktu swietlnego w dol
		lightPos[1] += -10;
		break;
	case '3': // Przesuniecie punktu swietlnego w prawo
		lightPos[0] += -10;
		break;
	case '4': // Przesuniecie punktu swietlnego w lewo
		lightPos[0] += 10;
		break;
	case '5': // Przesuniecie punktu swietlnego do siebie
		lightPos[2] += -10;
		break;
	case '6': // Przesuniecie punktu swietlnego w glab
		lightPos[2] += 10;
		break;
	case 'w': // Krok do przodu
		if( play )
		{
			switch( direction % 4 )
			{
			case 0:
				if( iz - 1 > -1 )
				{
					if( !own[iz-1][ix] )
					{
						iz -= 1;
						dz -= mnoznikz;
					}
				}
				break;
			case 1:
				if( ix + 1 < f_x )
				{
					if( !own[iz][ix+1] )
					{
						ix += 1;
						dx += mnoznikx;
					}
				}
				break;
			case 2:
				if( iz + 1 < f_z )
				{
					if( !own[iz+1][ix] )
					{
						iz += 1;
						dz += mnoznikz;
					}
				}
				break;
			case 3:
				if( ix - 1 > - 1 )
				{
					if( !own[iz][ix - 1] )
					{
						ix -= 1;
						dx -= mnoznikx;
					}
				}
				break;
			}
		}
		
		//
		break;
	case 's': // Krok do tylu
		if( play )
		{
			switch( (direction+2) % 4 )
			{
			case 0:
				if( iz - 1 > -1 )
				{
					if( !own[iz - 1][ix] )
					{
						iz -= 1;
						dz -= mnoznikz;
					}
				}
				break;
			case 1:
				if( ix + 1 < f_x )
				{
					if( !own[iz][ix + 1] )
					{
						ix += 1;
						dx += mnoznikx;
					}
				}
				break;
			case 2:
				if( iz + 1 < f_z )
				{
					if( !own[iz + 1][ix] )
					{
						iz += 1;
						dz += mnoznikz;
					}
				}
				break;
			case 3:
				if( ix - 1 > -1 )
				{
					if( !own[iz][ix - 1] )
					{
						ix -= 1;
						dx -= mnoznikx;
					}
				}
				break;
			}
		}
		break;
	case 'a': // obrot ekranu z zegarem
		direction - 1 < 0 ? direction = 999 : direction -= 1;
		kameraX -= 90;
		break;
	case 'd': // obrot ekranu przeciw zegarowi
		direction += 1;
		kameraX += 90;
		break;
	case 'r': // miekki reset
		direction = 0;
		kameraX = 180;
		break;
	case 't': // korekta wysokosci w dol
		dy += 0.01;
		break;
	case 'g': // korekta wysokci w gore
		dy -= 0.01;
		break;
	case 'f': // korekta glebokosci do siebie
		dz += 0.002;
		break;
	case 'h':
		dz -= 0.002; // korekta glebosko w glab
		break;
	case 'z': // wygaszanie koloru czerwonego
		if (lr > 0) lr -= 0.01;
		break;
	case 'x': // wygaszanie koloru zielonego
		if( lg > 0 ) lg -= 0.01;
		break;
	case 'c': // wygaszanie koloru niebieskiego
		if( lb > 0 ) lb -= 0.01;
		break;
	case 'v': // wznawianie koloru czerwonego
		if( lr < 1 ) lr += 0.01;
		break;
	case 'b': // wznawianie koloru zielonego
		if( lg < 1 ) lg += 0.01;
		break;
	case 'n': // wznawianie koloru niebieskiego
		if( lb < 1 ) lb += 0.01;
		break;

	}
	
}
/*###############################################################*/
void rysuj(void)
{
	// zmienne pomocnicze
	int skalar_f_x = int( f_x / 10 )+1;
	int skalar_f_z = int( f_z / 10 )+1;

	// kolor tla
	glClearColor(0.5f, 0.5, 0.5, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID); //u┐yj programu, czyli naszego shadera	
	
	GLuint MVP_id = glGetUniformLocation(programID, "MVP"); // pobierz lokalizację zmiennej 'uniform' "MV" w programie

	MV = glm::mat4( 1.0f );  //macierz jednostkowa
	MV = glm::translate( MV, glm::vec3( 0, -1, kameraD ) );

	//Wstepne przeksztalcenia
	MV = glm::rotate( MV, (float)glm::radians( kameraZ ), glm::vec3( 1, 0, 0 ) );
	MV = glm::rotate( MV, (float)glm::radians( kameraX), glm::vec3( 0, 1, 0 ) );
	MV = glm::scale( MV, glm::vec3( 100.0f*f_x/skalar_f_x, 100.0f, 100.0f*f_z/skalar_f_z ) );
	MV = glm::translate( MV, glm::vec3( dx, dy, dz ) );
	glm::mat4 MVP = P * MV;
	glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVP[0][0] ) );
	
	////////////////////////////////////////////////////

	//parametry materialu podlogi i oświetlenia
	glUniform3f(objectColor_id, 1.0f, 0.5f, 0.31f);
	glUniform3f(lightColor_id, lr, lg, lb);
	glUniform3f(lightPos_id,lightPos[0], lightPos[1], lightPos[2]);
	glUniform3f( materialambient_id, 0.0f, 0.1f, 0.06f );
	glUniform3f( materialdiffuse_id, 0.0f, 0.50980392f, 0.50980392f );
	glUniform3f( materialspecular_id, 0.50196078f, 0.50196078f, 0.50196078f );
	glUniform1f( materialshininess_id, 32.0f );
	glUniform3f( viewPos_id, kameraZ, kameraX, kameraD );

	/////////////////////////////////////////////////
	
	//Render the floor

	glBindVertexArray(FloorVao);

	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);

	//skalowanie obiektu sciany
	MV = glm::translate( MV, glm::vec3( 1, 1.0, 0.0 ) );
	MV = glm::scale( MV, glm::vec3( 1/f_x/3, 0.5, 1/f_z/3 ) );
	MV = glm::translate( MV, glm::vec3( -1, -1, 0 ) );
	MVP = P * MV;
	glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVP[0][0] ) );

	//parametry materialu sciany
	glUniform3f( materialambient_id, 0.05f, 0.05f, 0.05f );
	glUniform3f( materialdiffuse_id, 0.5f, 0.5f, 0.5f );
	glUniform3f( materialspecular_id, 0.7f, 0.7f, 0.7f );
	glUniform1f( materialshininess_id, 10.0f );
	glUniform3f( viewPos_id, kameraZ, kameraX, kameraD );

	//Proces rozmieszczania scian labiryntu wierz po wierszu
	MV = glm::translate( MV, glm::vec3(  -1, 0, -1 ) );
	for( int j = 0; j < f_z; j++ )
	{
		for( int i = 0; i < f_x; i++ )
		{
			MV = glm::translate( MV, glm::vec3( -i*3, 0, 0.0 ) );
			MVP = P * MV;
			glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVP[0][0] ) );
			if( own[j][i] )
			{
				glDrawElements( GL_QUADS, 24, GL_UNSIGNED_INT, 0 );

				MV = glm::translate( MV, glm::vec3( -1, 0, 0.0 ) );
				MVP = P * MV;
				glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVP[0][0] ) );
				if (i+1 < f_x) if( own[j][i+1] )glDrawElements( GL_QUADS, 24, GL_UNSIGNED_INT, 0 );
				MV = glm::translate( MV, glm::vec3( 1, 0, 0.0 ) );

				MV = glm::translate( MV, glm::vec3( 1, 0, 0.0 ) );
				MVP = P * MV;
				glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVP[0][0] ) );
				if( i - 1 > -1 ) if( own[j][i-1] )glDrawElements( GL_QUADS, 24, GL_UNSIGNED_INT, 0 );
				MV = glm::translate( MV, glm::vec3( -1, 0, 0.0 ) );

				MV = glm::translate( MV, glm::vec3( 0, 0, 1.0 ) );
				MVP = P * MV;
				glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVP[0][0] ) );
				if( j - 1 > -1 ) if( own[j-1][i] )glDrawElements( GL_QUADS, 24, GL_UNSIGNED_INT, 0 );
				MV = glm::translate( MV, glm::vec3( 0, 0, -1.0 ) );

				MV = glm::translate( MV, glm::vec3( 0, 0, -1.0 ) );
				MVP = P * MV;
				glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVP[0][0] ) );
				if( j + 1 < f_z ) if( own[j + 1][i] )glDrawElements( GL_QUADS, 24, GL_UNSIGNED_INT, 0 );
				MV = glm::translate( MV, glm::vec3( 0, 0, 1.0 ) );
			}

			MV = glm::translate( MV, glm::vec3( i*3, 0, 0.0 ) );
		}
		MV = glm::translate( MV, glm::vec3( 0, 0, -1*3 ) );
	}
	
	//Przygotowanie i umieszczenie swiatla
	MV = glm::scale( MV, glm::vec3(  0.01f * skalar_f_x*3 , .01f*2, 0.01f * skalar_f_z*3  ) );
	MVP = P * MV;
	glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVP[0][0] ) );

	glUseProgram(lamp_ID);
	GLuint MVPlamp_id = glGetUniformLocation(lamp_ID, "MVP");
	MV = glm::translate(MV, lightPos);
	MV = glm::scale(MV, glm::vec3(0.1f, 0.1f, 0.1f));
	MV = glm::rotate(MV, (float)glm::radians(-30.0), glm::vec3(0, 1, 0));
	 MVP = P * MV;
	glUniformMatrix4fv(MVPlamp_id, 1, GL_FALSE, &(MVP[0][0]));
	glBindVertexArray(lightVao);
	
	glDrawElements(GL_TRIANGLES, n_el, GL_UNSIGNED_INT, 0);

	glFlush();
	glutSwapBuffers();

}
/*###############################################################*/
void rozmiar(int width, int height)
{
	screen_width = width;
	screen_height = height;

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screen_width, screen_height);

	P = glm::perspective(glm::radians(60.0f), (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 1000.0f);

	glutPostRedisplay(); // Przerysowanie sceny
}

/*###############################################################*/
void idle()
{

	glutPostRedisplay();
}

/*###############################################################*/


void timer(int value) {

	
glutTimerFunc(20, timer, 0);
}
/*###############################################################*/



int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Labirynt");

	glewInit(); //init rozszerzeszeń OpenGL z biblioteki GLEW

	glutDisplayFunc(rysuj);			// def. funkcji rysuj¦cej
	glutIdleFunc(idle);			// def. funkcji rysuj¦cej w czasie wolnym procesoora (w efekcie: ci¦gle wykonywanej)
	//glutTimerFunc(20, timer, 0);
	glutReshapeFunc(rozmiar); // def. obs-ugi zdarzenia resize (GLUT)

	glutKeyboardFunc(klawisz);		// def. obsługi klawiatury
	glutMouseFunc(mysz); 		// def. obsługi zdarzenia przycisku myszy (GLUT)
	glutMotionFunc(mysz_ruch); // def. obsługi zdarzenia ruchu myszy (GLUT)


	glEnable(GL_DEPTH_TEST);

	//Czytanie obiektu swietlnego
	std::ifstream file("sphere.txt");
	if (file.fail())
	{
		printf("Cannot open sphere file or is not in this directory ! \n");
		system("pause");
		exit(-4);
	}

	if (file.is_open())
	{
		file >> n_v;
		file >> n_el;
		n_v = n_v * 3;
		
		vertices = (float*)calloc(n_v, sizeof(float));

		elements = (GLuint*)calloc(n_el, sizeof(int));

		for(int i=0; i < n_v; i++)
			file >> vertices[i];

		for (int i = 0; i < n_el; i++)
			file >> elements[i];
	
		normals = vertices;
	}

	//Czytanie labiryntu 
	std::ifstream file2( NAZWA_LABIRYNTU );
	if( file2.fail() )
	{
		printf( "Cannot open maze file or is not in this directory ! \n" );
		system( "pause" );
		exit( -4 );
	}

	if( file2.is_open() )
	{
		file2 >> f_x;
		file2 >> f_z;
		own = (int**)malloc( f_z * sizeof( int* ) );

		for( int i = 0; i < f_z; i++ ) own[i] = (int*)calloc( f_x, sizeof( int ) );

		for( int j = 0; j < f_z; j++ )
		{
			for( int i = 0; i < f_x; i++ )
			{
				file2 >> own[j][i];
			}
		}
		mnoznikz = 1.0 / f_z;
		mnoznikx = 1.0 / f_x;
	}

	programID = loadShaders("vertex_shader.glsl", "fragment_shader.glsl");
 

	glUseProgram(programID);

	objectColor_id = glGetUniformLocation(programID, "objectColor");
	lightColor_id = glGetUniformLocation(programID, "lightColor");
    lightPos_id = glGetUniformLocation(programID, "lightPos");
	materialshininess_id = glGetUniformLocation( programID, "material.shininess" );
	materialambient_id = glGetUniformLocation( programID, "material.ambient" );
	materialdiffuse_id = glGetUniformLocation( programID, "material.diffuse" );
	materialspecular_id = glGetUniformLocation( programID, "material.specular" ); 
	viewPos_id = glGetUniformLocation( programID, "viewPos" );

	glGenBuffers(1, &lightVbo);
	glBindBuffer(GL_ARRAY_BUFFER, lightVbo);
	glBufferData(GL_ARRAY_BUFFER, n_v * sizeof(float), vertices, GL_STATIC_DRAW);


	glGenBuffers(1, &lightEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_el * sizeof(int), elements, GL_STATIC_DRAW);

	//==================================================================================

	glGenBuffers( 1, &FloorVbo );
	glBindBuffer( GL_ARRAY_BUFFER, FloorVbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( f_vertices ), f_vertices, GL_STATIC_DRAW );

	glGenBuffers( 1, &FloorEbo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, FloorEbo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( f_elements ), f_elements, GL_STATIC_DRAW );

	glGenBuffers( 1, &FloorNormal );
	glBindBuffer( GL_ARRAY_BUFFER, FloorNormal );
	glBufferData( GL_ARRAY_BUFFER, sizeof( f_normals ), f_normals, GL_STATIC_DRAW );

	glGenVertexArrays(1, &FloorVao);

	glBindVertexArray(FloorVao);

	glBindBuffer(GL_ARRAY_BUFFER, FloorNormal);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, FloorVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, FloorEbo);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);


	// second, configure the light's VAO (lightVbo stays the same; the vertices are the same for the light object which is also a 3D sphere)
	lamp_ID = loadShaders("lamp_vshader.glsl", "lamp_fshader.glsl");
	glGenVertexArrays(1, &lightVao);
	glBindVertexArray(lightVao);
	glBindBuffer(GL_ARRAY_BUFFER, lightVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEbo);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glutMainLoop();					// start

	glDeleteVertexArrays(1, &lightVao);
	glDeleteBuffers(1, &lightVbo);
	glDeleteBuffers( 1, &lightEbo );

	glDeleteBuffers( 1, &FloorEbo );
	glDeleteBuffers( 1, &FloorVbo );
	glDeleteBuffers( 1, &FloorVao );
	glDeleteBuffers( 1, &FloorNormal );

	return(0);

}

