/*******************************************************************
		   Hierarchical Multi-Part Model Example
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "cube.h"
#include "QuadMesh.h"
#define PI 3.14159265358979323846

const int vWidth = 1000;    // Viewport width in pixels
const int vHeight = 800;    // Viewport height in pixels

float robotBodyWidth = 10.0;
float robotBodyLength = 4.0;
float robotBodyDepth = 5.0;
float topBodyLength = 0.03*robotBodyWidth;
float wheelLength = 0.25*robotBodyWidth;
float spinnerLength = 0.5*robotBodyWidth;

// Control robot body rotation on base
float robotAngle = 0.0;

// Control spinner angle
float spinnerAngle = 0.0;

// Control wheel angle, independent of each other
float leftWheelAngle = 0.0;
float rightWheelAngle = 0.0;

// Lighting/shading and material properties for robot
// Robot RGBA material properties
GLfloat robotBody_mat_ambient[] = { 0.24725f, 0.2245f, 0.0645f, 1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.34615f, 0.3143f, 0.0903f, 1.0f };
GLfloat robotBody_mat_specular[] = { 0.797357f, 0.723991f, 0.208006f, 1.0f };
GLfloat robotBody_mat_shininess[] = { 83.2f };

GLfloat robotTopBody_mat_ambient[] = { 0.02f, 0.02f, 0.02f, 1.0f };
GLfloat robotTopBody_mat_diffuse[] = { 0.01f, 0.01f, 0.01f, 1.0f };
GLfloat robotTopBody_mat_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat robotTopBody_mat_shininess[] = { 10.2F };

GLfloat robotWheel_mat_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
GLfloat robotWheel_mat_diffuse[] = { 0.01f, 0.01f, 0.01f, 1.0f };
GLfloat robotWheel_mat_specular[] = { 0.50f, 0.50f, 0.50f, 1.0f };
GLfloat robotWheel_mat_shininess[] = { 33.2f };

GLfloat robotSpinner_mat_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
GLfloat robotSpinner_mat_diffuse[] = { 0.50754f, 0.50754f, 0.50754f, 1.0f };
GLfloat robotSpinner_mat_specular[] = { 0.508273f, 0.508273f, 0.508273f, 1.0f };
GLfloat robotSpinner_mat_shininess[] = { 51.2F };

// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Mouse button
int currentButton;

// A template cube mesh
CubeMesh *cubeMesh = createCubeMesh();

// A flat open mesh
QuadMesh *groundMesh = NULL;
QuadMesh *wallMesh = NULL;

// Default Mesh Size
int meshSize = 10;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void animationHandler(int param);
void drawRobot();
void drawBody();
void drawTopBody();
void drawBottomBody();
void drawLeftWheel();
void drawRightWheel();
void drawSpinner();
void drawTopTriangle();
void drawBottomTriangle();
void drawCylinder();

//Keep track of forwards vector that the robot is facing, uses sin and cos so values will be
//from -1 to 1, always a unit vector
VECTOR3D forwards = VECTOR3D(0.0f, 0.0f, 1.0f);

//Keep track of robotX and robotZ to translate the robot
float robotX = 0;
float robotZ = 0;

int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Assignment 1");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);   

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	
	// Set up ground quad mesh
	VECTOR3D groundOrigin = VECTOR3D(-100.0f, 0.0f, 100.0f);
	VECTOR3D groundDir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D groundDir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
	groundMesh = new QuadMesh(meshSize, 200.0);
	groundMesh->InitMesh(meshSize, groundOrigin, 200.0, 200.0, groundDir1v, groundDir2v);
	VECTOR3D groundAmbient = VECTOR3D(0.6f, 0.0f, 0.0f);
	VECTOR3D groundDiffuse = VECTOR3D(0.2f, 0.2f, 0.2f);
	VECTOR3D groundSpecular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float groundShininess = 0.05;
	groundMesh->SetMaterial(groundAmbient, groundDiffuse, groundSpecular, groundShininess);
	

	// Set up wall quad mesh
	VECTOR3D wallOrigin = VECTOR3D(-100.0f, 0.0f, -60.0f);
	VECTOR3D wallDir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D wallDir2v = VECTOR3D(0.0f, 1.0f, 0.0f);
	wallMesh = new QuadMesh(meshSize, 200.0);
	wallMesh->InitMesh(meshSize, wallOrigin, 200.0, 200.0, wallDir1v, wallDir2v);
	VECTOR3D wallAmbient = VECTOR3D(0.6f, 0.0f, 0.0f);
	VECTOR3D wallDiffuse = VECTOR3D(0.3f, 0.3f, 0.3f);
	VECTOR3D wallSpecular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float wallShininess = 0.05;
	wallMesh->SetMaterial(wallAmbient, wallDiffuse, wallSpecular, wallShininess);

}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Create Viewing Matrix V
	// Set up the camera at position (0, 20, 40) looking at the origin, up along positive y axis
	gluLookAt(0.0, 20.0, 40.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw Robot
	// Current transformation matrix is set to IV, where I is identity matrix
	// CTM = IV
	drawRobot();

	// Drawing a closed cube mesh
	glPushMatrix();

	glTranslatef(-12.0, -1.0, 7.0);
	glScalef(2.0f, 2.0f, 2.0f);
	drawCubeMesh(cubeMesh);

	glPopMatrix();

	glPushMatrix();
	glTranslatef(25.0, 1.5, -17.0);
	glScalef(4.0f, 4.0f, 4.0f);
	drawCubeMesh(cubeMesh);

	glPopMatrix();

	glPopMatrix();

	glPushMatrix();
	glTranslatef(25.0, 7.5, -17.0);
	glScalef(2.0f, 2.0f, 2.0f);
	drawCubeMesh(cubeMesh);

	glPopMatrix();
	
	// Draw ground
	glPushMatrix();

	glTranslatef(0.0, -2.5, 0.0);
	groundMesh->DrawMesh(meshSize);
	wallMesh->DrawMesh(meshSize);

	glPopMatrix();

	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawRobot()
{
	glPushMatrix();

	forwards.SetX(sin((PI / 180) * robotAngle));
	forwards.SetZ(cos((PI / 180) * robotAngle));

	glTranslatef(robotX, 0, robotZ);
	glRotatef(robotAngle, 0.0, 1.0, 0.0);
	
	drawBody();
	drawTopBody();
	drawBottomBody();
	drawLeftWheel();
	drawRightWheel();
	drawSpinner();
	drawTopTriangle();
	drawBottomTriangle();
	drawCylinder();

	glPopMatrix();
}


void drawBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	glPushMatrix();
	glScalef(robotBodyWidth, robotBodyLength, robotBodyDepth);
	glutSolidCube(1.0);
	glPopMatrix();
}

void drawTopBody()
{
	// Set robot material properties per body part. Can have seperate material properties for each part
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotTopBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotTopBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotTopBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotTopBody_mat_shininess);

	glPushMatrix();
	// Position head with respect to parent (body)
	glTranslatef(0, 0.5*robotBodyLength + 0.5*topBodyLength, 0); // this will be done last

	// Build Head
	glPushMatrix();
	//glScalef(0.4*robotBodyWidth, 0.4*robotBodyWidth, 0.4*robotBodyWidth);
	glScalef(robotBodyWidth, topBodyLength, robotBodyDepth);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

void drawBottomBody()
{
	// Set robot material properties per body part. Can have seperate material properties for each part
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotTopBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotTopBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotTopBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotTopBody_mat_shininess);

	glPushMatrix();
	// Position head with respect to parent (body)
	glTranslatef(0, -0.5*robotBodyLength - 0.5*topBodyLength, 0); // this will be done last

	// Build Head
	glPushMatrix();
	//glScalef(0.4*robotBodyWidth, 0.4*robotBodyWidth, 0.4*robotBodyWidth);
	glScalef(robotBodyWidth, topBodyLength, robotBodyDepth);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

void drawLeftWheel()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotWheel_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotWheel_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotWheel_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotWheel_mat_shininess);

	GLUquadricObj *myCylinder;
	myCylinder = gluNewQuadric();
	gluQuadricDrawStyle(myCylinder, GLU_FILL);

	GLUquadricObj *myDisk;
	myDisk = gluNewQuadric();
	gluQuadricDrawStyle(myDisk, GLU_FILL);

	glPushMatrix();
	glRotatef(leftWheelAngle, 1, 0, 0);

	//Rotate wheel to be perpendicular to robot body
	glPushMatrix();
	glTranslatef(0.5*robotBodyWidth + 0.5*wheelLength, 0, 0.0);
	glRotatef(90, 0.0, 1.0, 0.0);
	glTranslatef(-0.5*robotBodyWidth - 0.5*wheelLength, 0, 0.0);
	
	//Position arm with respect to parent body
	glPushMatrix();
	glTranslatef(0.5*robotBodyWidth + 0.5*wheelLength, 0.0, -0.5*wheelLength);
	
	//Create cylinder and scale the cylinder
	glPushMatrix();
	glScalef(wheelLength, wheelLength, 0.7*wheelLength);
	gluCylinder(myCylinder, 1, 1, 1, 100, 100);
	
	//Create disk for wheel
	glPushMatrix();
	glTranslatef(0, 0, 0.4*wheelLength);
	gluDisk(myDisk, 0, 1, 100, 100);

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotTopBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotTopBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotTopBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotTopBody_mat_shininess);
	glScalef(1.7*(1 / wheelLength), 1.7*(1 / wheelLength), 1 / (0.8*wheelLength));
	glutSolidCube(1);

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}

void drawRightWheel()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotWheel_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotWheel_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotWheel_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotWheel_mat_shininess);

	GLUquadricObj *myCylinder;
	myCylinder = gluNewQuadric();
	gluQuadricDrawStyle(myCylinder, GLU_FILL);

	GLUquadricObj *myDisk;
	myDisk = gluNewQuadric();
	gluQuadricDrawStyle(myDisk, GLU_FILL);

	glPushMatrix();
	glRotatef(rightWheelAngle, 1, 0, 0);

	//Rotate wheel to be perpendicular to robot body
	glPushMatrix();
	glTranslatef(-0.5*robotBodyWidth - 0.5*wheelLength, 0, 0.0);
	glRotatef(-90, 0.0, 1.0, 0.0);
	glTranslatef(0.5*robotBodyWidth + 0.5*wheelLength, 0, 0.0);

	//Position arm with respect to parent body
	glPushMatrix();
	glTranslatef(-0.5*robotBodyWidth - 0.5*wheelLength, 0.0, -0.5*wheelLength);

	//Create cylinder and scale the cylinder
	glPushMatrix();
	glScalef(wheelLength, wheelLength, 0.7*wheelLength);
	gluCylinder(myCylinder, 1, 1, 1, 100, 100);

	//Create disk for wheel
	glPushMatrix();
	glTranslatef(0, 0, 0.4*wheelLength);
	gluDisk(myDisk, 0, 1, 100, 100);

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotTopBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotTopBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotTopBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotTopBody_mat_shininess);
	glScalef(1.7*(1/wheelLength), 1.7*(1/wheelLength), 1/(0.8*wheelLength));
	glutSolidCube(1);
	
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}

void drawSpinner()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotSpinner_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotSpinner_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotSpinner_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotSpinner_mat_shininess);

	GLUquadricObj *myCylinder;
	myCylinder = gluNewQuadric();
	gluQuadricDrawStyle(myCylinder, GLU_FILL);

	GLUquadricObj *myDisk;
	myDisk = gluNewQuadric();
	gluQuadricDrawStyle(myDisk, GLU_FILL);

	//Create cylinder and scale
	glPushMatrix();

	glTranslatef(0, 0.5, 8);
	glRotatef(spinnerAngle, 0.0, 1.0, 0.0);
	glTranslatef(0, -0.5, -8);
	glTranslatef(0, 0.5, 8);
	glRotatef(90, 1.0, 0.0, 0.0);
	glTranslatef(0, -0.5, -8);
	glTranslatef(0, 0.5, 8);
	glScalef(spinnerLength, spinnerLength, 0.2*spinnerLength);
	gluCylinder(myCylinder, 1, 1, 1, 20, 20);

	//Draw top disk of spinner
	glPushMatrix();
	glRotatef(180, 1.0, 0.0, 0.0);
	gluDisk(myDisk, 0, 1, 20, 20);
	
	glPopMatrix();
	glPopMatrix();
}

void drawTopTriangle()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotTopBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotTopBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotTopBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotTopBody_mat_shininess);

	glPushMatrix();
	glTranslatef(0, 0.5*robotBodyLength + topBodyLength, 0.5*robotBodyDepth);

	glScalef(0.5*robotBodyWidth, 10, 8);
	//Top piece
	glBegin(GL_TRIANGLES);
		glNormal3f(0, 1, 0);
		glVertex3f(1, 0, 0);
		glNormal3f(0, 1, 0);
		glVertex3f(-1, 0, 0);
		glNormal3f(0, 1, 0);
		glVertex3f(0, 0, 1);

		glVertex3f(0, 0, 1);
		glVertex3f(0, -0.03, 1);
		glVertex3f(-1, 0, 0);

		glVertex3f(-1, -0.03, 0);
		glVertex3f(-1, 0, 0);
		glVertex3f(0, -0.03, 1);

		glVertex3f(0, 0, 1);
		glVertex3f(0, -0.03, 1);
		glVertex3f(1, 0, 0);

		glVertex3f(1, -0.03, 0);
		glVertex3f(1, 0, 0);
		glVertex3f(0, -0.03, 1);
	glEnd();
	
	glPopMatrix();
}

void drawBottomTriangle()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotTopBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotTopBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotTopBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotTopBody_mat_shininess);

	glPushMatrix();
	glTranslatef(0, -0.5*robotBodyLength, 0.5*robotBodyDepth);

	glScalef(0.5*robotBodyWidth, 10, 8);
	//Top piece
	glBegin(GL_TRIANGLES);
	glNormal3f(0, 1, 0);
	glVertex3f(1, 0, 0);
	glNormal3f(0, 1, 0);
	glVertex3f(-1, 0, 0);
	glNormal3f(0, 1, 0);
	glVertex3f(0, 0, 1);

	glVertex3f(0, 0, 1);
	glVertex3f(0, -0.03, 1);
	glVertex3f(-1, 0, 0);

	glVertex3f(-1, -0.03, 0);
	glVertex3f(-1, 0, 0);
	glVertex3f(0, -0.03, 1);

	glVertex3f(0, 0, 1);
	glVertex3f(0, -0.03, 1);
	glVertex3f(1, 0, 0);

	glVertex3f(1, -0.03, 0);
	glVertex3f(1, 0, 0);
	glVertex3f(0, -0.03, 1);
	glEnd();

	glPopMatrix();
}

void drawCylinder() {
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	GLUquadricObj *myCylinder;
	myCylinder = gluNewQuadric();
	gluQuadricDrawStyle(myCylinder, GLU_FILL);

	GLUquadricObj *myDisk;
	myDisk = gluNewQuadric();
	gluQuadricDrawStyle(myDisk, GLU_FILL);

	//Draw cylinder to hold spinner
	glPushMatrix();

	glTranslatef(0, -2, 8);
	glRotatef(spinnerAngle, 0.0, 1.0, 0.0);
	glTranslatef(0, 2, -8);
	glTranslatef(0, -2, 8);
	glScalef(0.5, 5, 0.5);
	glTranslatef(0, 2, -8);
	glTranslatef(0, -2, 8);
	glRotatef(90, 0.0, 0.0, 1.0);
	glRotatef(90, 0.0, 1.0, 0.0);
	glTranslatef(0, 2, -8);
	glTranslatef(0, -2, 8);
	gluCylinder(myCylinder, 1, 1, 1, 15, 15);

	//Draw disk cap on spinner
	glPushMatrix();

	glTranslatef(0, 0, 1);
	gluDisk(myDisk, 0, 1, 100, 100);

	glPushMatrix();

	glTranslatef(0, 0, -0.5);
	glScalef(10, 10, 0.1);
	glutSolidCube(1);

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool spinnerStop = true;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case ' ':
		glutTimerFunc(10, animationHandler, 0);
		spinnerStop = !spinnerStop;
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


void animationHandler(int param)
{
	if (!spinnerStop)
	{
		spinnerAngle += 5;
		glutPostRedisplay();
		glutTimerFunc(10, animationHandler, 0);
	}
}



// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1)
	{
		printf("CONTROLS\n");
		printf("========================================\n");
		printf("Use left arrow key to rotate counter-clockwise\n");
		printf("Use right arrow key to rotate clockwise\n");
		printf("Use up arrow key to increment forwards\n");
		printf("Use down arrow key to increment backwards\n");
		printf("Use spacebar to turn the spinner on or off\n");
		printf("\n");
	}
	// Do transformations with arrow keys
	// GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
	else if (key == GLUT_KEY_RIGHT)   
	{
		leftWheelAngle += 8;
		rightWheelAngle -= 8;
		robotAngle -= 3.0;
	}
	else if (key == GLUT_KEY_LEFT)
	{
		leftWheelAngle -= 8;
		rightWheelAngle += 8;
		robotAngle += 3.0;
	}
	else if (key == GLUT_KEY_UP)
	{
		leftWheelAngle += 8;
		rightWheelAngle += 8;
		robotX += forwards.GetX();
		robotZ += forwards.GetZ();
	}
	else if (key == GLUT_KEY_DOWN)
	{
		leftWheelAngle -= 8;
		rightWheelAngle -= 8;
		robotX -= forwards.GetX();
		robotZ -= forwards.GetZ();
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;

		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

