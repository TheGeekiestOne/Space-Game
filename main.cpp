
/*********************************
******     Copyright (c)    ******
***        Ayran Olckers       ***
**           W1654684           **
**           12/2019            **
******                      ******
**********************************/


#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0

#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include "gltools.h"
#include "freeglut.h"
//#include
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

typedef struct point3D {
	float x;
	float y;
	float z;
}point3D;

#define numTorous 5
point3D pointsForTorous[numTorous] = {{25.0f, 15.0f, -60.0f},{30.0f, 5.0f, -75.0f},{-20.0f, 0.0f, -70.0f},{0.0f, -15.0f, -80.0f},{21.2f, 0.0f, -30.0f}};

float Angle = 0.0f, Angle2 = 0.0f, step = 0.25f;
float AngleHoop = 0.0f, AnglePlanet = 0.0f;
point3D shipPosition = {0.0f, 0.0f, 0.0f}; // player position
int showhud = 0;
int Wwidth = 800, Wheight = 600;
int collected = 0; // number of collected crystals

//Makes the image into a texture
void loadTexture(const char *filename, GLuint textureId)
{
	GLint iWidth, iHeight, iComponents;
	GLenum eFormat;
	GLbyte *pBytes = gltLoadTGA(filename,&iWidth, &iHeight, &iComponents, &eFormat);
	glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
}

#define TEXTURE_COUNT 4
GLuint textures[TEXTURE_COUNT];
GLUquadric *quad[2];

void SetupRC() {
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glGenTextures(TEXTURE_COUNT, textures); //Make room for our texture
	loadTexture("jupiterC.tga", textures[0]);
	loadTexture("tycho_cyl_glow.tga", textures[1]);
	loadTexture("starField.tga", textures[2]);
	loadTexture("hud.tga", textures[3]);
	quad[0] = gluNewQuadric();
	gluQuadricTexture(quad[0], 1);
	quad[1] = gluNewQuadric();
	gluQuadricTexture(quad[1], 1);
}

// from Appendix B
void setOrthographicProjection() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, Wwidth, 0, Wheight);
	glScalef(1, -1, 1);
	glTranslatef(0, -Wheight, 0);
	glMatrixMode(GL_MODELVIEW);
}

// from Appendix B
void resetPerspectiveProjection() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void ChangeSize(int w, int h)
{
	GLfloat fAspect;
	
	// Prevent a divide by zero
	if(h == 0)
			h = 1;
	
	// Set Viewport to window dimensions
	glViewport(0, 0, w, h);
	
	// Calculate aspect ratio of the window
	fAspect = (GLfloat)w/(GLfloat)h;
	
	// Set the perspective coordinate system
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// field of view of 45 degrees, near and far planes 1.0 and 1000
	//that z-Near and z-Far should typically have a ratio of 1000:1 to make sorting out z depth easier for the GPU
	gluPerspective(55.0f, fAspect, 1.0, 1000.0); //may need to make larger depending on project
	// Modelview matrix reset
	glMatrixMode(GL_MODELVIEW);
	//pull the eye position back to view the scene
	gluLookAt(0.00,0.00,400.0,//eye
			0.00,0.00,0.00,//centre
			0.00,1.00,0.00);//up
	Wwidth = w;
	Wheight = h;
}

void drawHUD()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(255, 255, 255, 128); // white, alpha is 128/255
	glEnable( GL_TEXTURE_2D );
	//bind the texture 
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glBegin(GL_QUADS);
	//glNormal3f(0.0f, 0.0f, 1.0f);
	// Error in the function gltLoadTGA. It seems that works if the width is multiple of 4
	//glTexCoord2f(-0.375f, 1.0f); // with original hud.tga 1422x800
	glTexCoord2f(0.0f, 1.0f); // changing the size of hud.tga to 1424x800
	glVertex3f(0.0f, 0.0f, 1.0f);
	//glTexCoord2f(0.625f, 1.0f); // with original hud.tga
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(Wwidth, 0.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(Wwidth, Wheight,1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0.0f, Wheight, 1.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void drawstring(const char *s, void *font)// write text on glut window.
{
	while (*s)
		glutBitmapCharacter(font,*(s++));
}


void RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glColor3ub(255, 255, 255); // white
	glRotatef(-Angle, 0.0f, 1.0f, 0.0f); // change the xz player direction
	{ float angle_rad = Angle*float(M_PI/180);
	glRotatef(-Angle2, sinf(angle_rad), 0.0f, cosf(angle_rad)); } //correct rotation axis.
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glPushMatrix();
		glRotatef(90,1.0f,0.0f,0.0f); // the planet poles are in the correct position
		gluSphere(quad[1],500,40,40); // star map sphere
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glTranslatef(0.0f, -460.0f, 0.0f);
		glRotatef(AnglePlanet,0.0f,0.0f,1.0f);
		gluSphere(quad[0],30,20,20); // planet
	glPopMatrix();
	glTranslatef(-shipPosition.x, -shipPosition.y, -shipPosition.z); // move the player (actually the scene is moved)
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	for (int i = 0; i < numTorous; ++i)
	{
		glPushMatrix();
			glTranslatef(pointsForTorous[i].x, pointsForTorous[i].y, pointsForTorous[i].z);
			glRotated(-AngleHoop, 0.0f, 1.0f, 0.0f);
			glutSolidTorus(0.25f, 3.0f, 10, 40);
		glPopMatrix();
	}
	glDisable(GL_TEXTURE_2D);
	glColor3ub(255, 255, 0); // yellow
	for (int i = collected; i < numTorous; ++i) // the collected crystals are not displayed
	{
		glPushMatrix();
			glTranslatef(pointsForTorous[i].x, pointsForTorous[i].y, pointsForTorous[i].z);
			glRotated(AngleHoop, 0.0f, 1.0f, 0.0f);
			glutSolidIcosahedron();
		glPopMatrix();
		glColor3ub(0, 255, 0); // green
	}
	setOrthographicProjection();
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	if (showhud)
		drawHUD();
	glColor3ub(0, 255, 0); // green
	glRasterPos2f(10, 40);
	{ ostringstream os;
		os << "Crystals Collected: " << collected;
		drawstring(os.str().c_str(), GLUT_BITMAP_TIMES_ROMAN_24);
	}
	glColor3ub(255, 0, 0); // red
	glRasterPos2f(10, 70);
	{ ostringstream os;
		os << "Ship Position X: " << shipPosition.x;
		drawstring(os.str().c_str(), GLUT_BITMAP_HELVETICA_12);
	}
	glColor3ub(0, 255, 0); // green
	glRasterPos2f(10, 100);
	{ ostringstream os;
		os << "Ship Position Y: " << shipPosition.y;
		drawstring(os.str().c_str(), GLUT_BITMAP_HELVETICA_12);
	}
	glColor3ub(64, 64, 255); // blue
	glRasterPos2f(10, 130);
	{ ostringstream os;
		os << "Ship Position Z: " << shipPosition.z;
		drawstring(os.str().c_str(), GLUT_BITMAP_HELVETICA_12);
	}
	glColor3ub(255, 255, 255); // white
	glRasterPos2f(10, 160);
	{ ostringstream os;
		os << "Rotation X: " << 0;
		drawstring(os.str().c_str(), GLUT_BITMAP_HELVETICA_12);
	}
	glRasterPos2f(10, 180);
	{ ostringstream os;
		os << "Rotation Y: " << Angle;
		drawstring(os.str().c_str(), GLUT_BITMAP_HELVETICA_12);
	}
	glRasterPos2f(10, 200);
	{ ostringstream os;
		os << "Rotation Z: " << Angle2;
		drawstring(os.str().c_str(), GLUT_BITMAP_HELVETICA_12);
	}
	glEnable(GL_DEPTH_TEST);
	resetPerspectiveProjection();
	glutSwapBuffers();
}


void TimerFunc(int value)
{
	AngleHoop += 1.0f;
	if(AngleHoop > 180.f)
			AngleHoop -= 360;
	AnglePlanet += 0.25f;
	if(AnglePlanet > 180.f)
			AnglePlanet -= 360;
	glutPostRedisplay();
	glutTimerFunc(25,TimerFunc,0); // wait 25 ms
}

void resetgame()
{
	Angle = Angle2 = 0.0f;
	shipPosition.x = shipPosition.y = shipPosition.z = 0.0f;
	collected = 0;
}

void testCollision()
{
	if (collected < numTorous)
	{
		float x = shipPosition.x - pointsForTorous[collected].x;
		float y = shipPosition.y - pointsForTorous[collected].y;
		float z = shipPosition.z - pointsForTorous[collected].z;
		if (x*x + y*y + z*z < 1.5f*1.5f)
			++collected;
	}
	else
		resetgame();
}

void processsArrowKeys(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:
		Angle2 += 0.5f;
		if (Angle2 > 180.0f) Angle2 -= 360.0f;
		break;
	case GLUT_KEY_UP:
		shipPosition.x -= step*cosf(Angle*float(M_PI/180))*sinf(Angle2*float(M_PI/180));
		shipPosition.z += step*sinf(Angle*float(M_PI/180))*sinf(Angle2*float(M_PI/180));
		shipPosition.y += step*cosf(Angle2*float(M_PI/180));
		testCollision();
		break;
	case GLUT_KEY_RIGHT:
		Angle2 -= 0.5f;
		if (Angle2 < -180.0f) Angle2 += 360.0f;
	break;
	case GLUT_KEY_DOWN:
		shipPosition.x += step*cosf(Angle*float(M_PI/180))*sinf(Angle2*float(M_PI/180));
		shipPosition.z -= step*sinf(Angle*float(M_PI/180))*sinf(Angle2*float(M_PI/180));
		shipPosition.y -= step*cosf(Angle2*float(M_PI/180));
		testCollision();
		break;
	}
}

void processKeys(unsigned char key, int x, int y)
{
	switch (key) {
	case 'W':
	case 'w':// forward
		shipPosition.x -= step*sinf(Angle*float(M_PI/180));
		shipPosition.z -= step*cosf(Angle*float(M_PI/180));
		testCollision();
		break;
	case 'S':
	case 's':// back
		shipPosition.x += step*sinf(Angle*float(M_PI/180));
		shipPosition.z += step*cosf(Angle*float(M_PI/180));
		testCollision();
		break;
	case 'A':
	case 'a':// left
		shipPosition.x -= step*cosf(Angle*float(M_PI/180))*cosf(Angle2*float(M_PI/180));
		shipPosition.z += step*sinf(Angle*float(M_PI/180))*cosf(Angle2*float(M_PI/180));
		shipPosition.y -= step*sinf(Angle2*float(M_PI/180));
		testCollision();
		break;
	case 'D':
	case 'd':// right
		shipPosition.x += step*cosf(Angle*float(M_PI/180))*cosf(Angle2*float(M_PI/180));
		shipPosition.z -= step*sinf(Angle*float(M_PI/180))*cosf(Angle2*float(M_PI/180));
		shipPosition.y += step*sinf(Angle2*float(M_PI/180));
		testCollision();
		break;
	case 'Z':
	case 'z':// rotate left
		Angle += 0.5f;
		if (Angle > 180.0f) Angle -= 360.0f;
		break;
	case 'X':
	case 'x':// rotate right
		Angle -= 0.5f;
		if (Angle < -180.0f) Angle += 360.0f;
		break;
	case 'R':
	case 'r':
		resetgame();
		break;
	case 'H':
	case 'h':
		showhud = !showhud;
		break;
	}
}


int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(Wwidth, Wheight);
	glutCreateWindow("Space crystals");
	glutSpecialFunc(processsArrowKeys);
	glutKeyboardFunc(processKeys);
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutTimerFunc(25,TimerFunc,0);
	SetupRC();
	cout << "SPACE GAME\n"
		<< "Created by Ayran Olckers\n"
		<< "========================================================\n"
		<< "Welcome to a 3D Space Game designed in OpenGL with GLUT\n"
		<< "The aim of the game is to move your first person character\n"
		<< "through space to collect the crystals within the spinning hoops\n\n"
		<< "When all crystals have been collected the game will reset\n\n";
	cout << "Controls:\n\nLeft Arrow = Roll the Ship Left\n"
		<< "Rigth Arrow = Roll the Ship Right\n"
		<< "Up Arrow = Move the Ship Up\n"
		<< "Down Arrow = Move the Ship Down\n"
		<< "S = Move the Ship Backwards\n"
		<< "W = Move the Ship Forwards\n"
		<< "D = Move the Ship Right\n"
		<< "A = Move the Ship Left\n"
		<< "Z = Rotate the Ship Left\n"
		<< "X = Rotate the Ship Right\n"
		<< "H = Turn the HUD ON/OFF\n"
		<< "R = Reset the game";
	glutMainLoop();
	return 0;
}

