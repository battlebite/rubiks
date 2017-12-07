
/*
Author: James Nichols
Project
Class: CSIT - 462 Computer Graphics Fall 2017
Uses: Code provided for assignment 2 and parts of code from assignment 1
	also inspired by/used parts of code from: 
	Source 1: https://www.experts-exchange.com/questions/22460524/Rubik's-Cube-creation-in-OpenGL-and-C.html
*/

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef _WIN32
#include <unistd.h>
#else
void usleep(unsigned int nanosec)
{
	Sleep(nanosec / 1000);
}
#endif


// *************** GLOBAL VARIABLES *************************


const float PI = 3.14159;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;               // Glut window ID (for display)
GLUI *glui;                 // Glui window (for controls)
int Win[2];                 // window (x,y) size
float rotation[3] = {0,0,0};

// ---------------- ANIMATION VARIABLES ---------------------

// Animation settings
int animate_mode = 0;       // 0 = no anim, 1 = animate
int animation_frame = 0;      // Specify current frame of animation

// Joint parameters
const float JOINT_MIN = -45.0f;
const float JOINT_MAX = 45.0f;
float joint_rot = 0.0f;

//////////////////////////////////////////////////////
// TODO: Add additional joint parameters here
//////////////////////////////////////////////////////

//Joints added with different values to check placement/setup
float joint_rot_2 = 10.0f;
float joint_rot_3 = -15.0f;
float joint_rot_4 = -35.0f;



// Variables for moving camera position and rotation
static int camAngleX = 0, camAngleY = 0, camAngleZ = 0;
static int camPosX = 0, camPosY = 0, camPosZ = 195;




// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initGlut(char* winName);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void myReshape(int w, int h);
void animate();
void display(void);

// Callback for handling events in glui
void GLUI_Control(int id);


// Functions to help draw the object
void drawSquare(float size);

//===============================FROM Source 1
void glDrawCube();
void buildCubeSolid(float x, float y, float z);
void buildCubeWire(float x, float y, float z);
//===============================


// Return the current system clock (in seconds)
double getTime();


// ******************** FUNCTIONS ************************



// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

	// Process program arguments
	if (argc != 3) {
		printf("Usage: demo [width] [height]\n");
		printf("Using 300x200 window by default...\n");
		Win[0] = 600;
		Win[1] = 400;
	}
	else {
		Win[0] = atoi(argv[1]);
		Win[1] = atoi(argv[2]);
	}


	// Initialize glut, glui, and opengl
	glutInit(&argc, argv);
	initGlut(argv[0]);
	initGlui();
	initGl();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	// Invoke the standard GLUT main event loop
	glutMainLoop();

	return 0;         // never reached
}


//Adds keyboard control to camera for assignment 3 task 3
//Unsure if position or rotation was to be changed, so both are implemented
//Keys rotate the camera, F-Keys move the camera
//Arrow keys left and right rotate camera left or right 5 degress per press
//Arrow keys up and down rotate camera up or down 5 degress per press
//Pageup and Pagedown rotate camera left or right 5 degress per press
//F1 moves camera to left, F2 moves camera to the right
//F3 moves camera downward, F4 moves camera upward
//F5 moves camera towards the scene, F6 moves camera way from the scene
void keyboard(unsigned char key, int, int) {
	switch (key) {
	case 'q':
		rotation[0] += 90;
		glutPostRedisplay();
		break;
	case 'e':
		rotation[0] -= 90;
		glutPostRedisplay();
		break;
	case 'a':
		rotation[1] += 90;
		glutPostRedisplay();
		break;
	case 'd':
		rotation[1] -= 90;
		glutPostRedisplay();
		break;
	case 'z':
		rotation[2] += 90;
		glutPostRedisplay();
		break;
	case 'c':
		rotation[2] -= 90;
		glutPostRedisplay();
		break;
	}
}
void special(int key, int, int) {
	int mod;
	switch (key) {
	case GLUT_KEY_LEFT: (camAngleY += 5) %= 360; break;
	case GLUT_KEY_RIGHT: (camAngleY -= 5) %= 360; break;
	case GLUT_KEY_UP: (camAngleX += 5) %= 360; break;
	case GLUT_KEY_DOWN: (camAngleX -= 5) %= 360; break;
	case GLUT_KEY_PAGE_DOWN: (camAngleZ += 5) %= 360; break;
	case GLUT_KEY_PAGE_UP: (camAngleZ -= 5) %= 360; break;
	case GLUT_KEY_F1: (camPosX += 5); break;
	case GLUT_KEY_F2: (camPosX -= 5); break;
	case GLUT_KEY_F3: (camPosY += 5); break;
	case GLUT_KEY_F4: (camPosY -= 5); break;
	case GLUT_KEY_F5: (camPosZ += 5); break;
	case GLUT_KEY_F6: (camPosZ -= 5); break;
		
	default: return;
	}
	glutPostRedisplay();
}

// Initialize glut and create a window with the specified caption 
void initGlut(char* winName)
{
	// Set video mode: double-buffered, color, depth-buffered
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// Create window
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(Win[0], Win[1]);
	windowID = glutCreateWindow(winName);

	// Setup callback functions to handle events
	glutReshapeFunc(myReshape); // Call myReshape whenever window resized
	glutDisplayFunc(display);   // Call display whenever new frame needed 
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);	// Called for camera movements
}


// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
	exit(0);
}



// Animate button handler.  Called when the "animate" checkbox is pressed.
void animateButton(int)
{
	// synchronize variables that GLUT uses
	glui->sync_live();

	animation_frame = 0;
	if (animate_mode == 1) {
		// start animation
		GLUI_Master.set_glutIdleFunc(animate);
	}
	else {
		// stop animation
		GLUI_Master.set_glutIdleFunc(NULL);
	}
}

// Initialize GLUI and the user interface
void initGlui()
{
	GLUI_Master.set_glutIdleFunc(NULL);

	// Create GLUI window
	glui = GLUI_Master.create_glui("Glui Window", 0, Win[0] + 10, 0);

	// Create a control to specify the rotation of the joint
	GLUI_Spinner *joint_spinner	= glui->add_spinner("Joint 1", GLUI_SPINNER_FLOAT, &joint_rot);
	joint_spinner->set_speed(0.1);
	joint_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

	///////////////////////////////////////////////////////////
	// TODO: 
	//   Add controls for additional joints here
	//	May use these for the rotion of slices in rubiks cube
	//	6 Spinners around the center cube on each face
	///////////////////////////////////////////////////////////

	GLUI_Spinner *joint_spinner_2 = glui->add_spinner("Joint 2", GLUI_SPINNER_FLOAT, &joint_rot_2);
	joint_spinner_2->set_speed(0.1);
	joint_spinner_2->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

	GLUI_Spinner *joint_spinner_3 = glui->add_spinner("Joint 3", GLUI_SPINNER_FLOAT, &joint_rot_3);
	joint_spinner_3->set_speed(0.4);
	joint_spinner_3->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

	GLUI_Spinner *joint_spinner_4 = glui->add_spinner("Joint 4", GLUI_SPINNER_FLOAT, &joint_rot_4);
	joint_spinner_4->set_speed(0.9);
	joint_spinner_4->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

	

	// Add button to specify animation mode 
	glui->add_separator();
	glui->add_checkbox("Animate", &animate_mode, 0, animateButton);

	// Add "Quit" button
	glui->add_separator();
	glui->add_button("Quit", 0, quitButton);

	// Set the main window to be the "active" window
	glui->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
	// glClearColor (red, green, blue, alpha)
	// Ignore the meaning of the 'alpha' value for now
	glClearColor(0.7f, 0.7f, 0.9f, 1.0f);
}




// Callback idle function for animating the scene
void animate()
{
	// Update geometry
	const double joint_rot_speed = 0.1;
	

	double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
	joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;

	

	///////////////////////////////////////////////////////////
	// TODO:
	//   Modify this function animate the character's joints
	//   Note: Nothing should be drawn in this function!  OpenGL drawing
	//   should only happen in the display() callback.
	///////////////////////////////////////////////////////////

	const double joint_rot_speed2 = 0.2;
	const double joint_rot_speed3 = 0.1;
	const double joint_rot_speed4 = 0.25;

	double joint_rot_t2 = (sin(animation_frame *joint_rot_speed2) + 1.0 / 2.0);
	joint_rot_2 = joint_rot_t2 * JOINT_MIN + (1 - joint_rot_t2) * JOINT_MAX;

	double joint_rot_t3 = (cos(animation_frame *joint_rot_speed) + 1.0 / 2.0);
	joint_rot_3 = joint_rot_t3 * JOINT_MIN + (1 - joint_rot_t3) * JOINT_MAX;

	double joint_rot_t4 = (sin(animation_frame *joint_rot_speed4) + 1.0 / 2.0);
	joint_rot_4 = joint_rot_t4 * JOINT_MIN + (1 - joint_rot_t4) * JOINT_MAX;

	// Update user interface
	glui->sync_live();

	// Tell glut window to update itself.  This will cause the display()
	// callback to be called, which renders the object (once you've written
	// the callback).
	glutSetWindow(windowID);
	glutPostRedisplay();

	// increment the frame number.
	animation_frame++;

	// Wait 50 ms between frames (20 frames per second)
	usleep(50000);
}


// Handles the window being resized by updating the viewport
// and projection matrices
void myReshape(int w, int h)
{
	// Setup projection matrix for new window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-w / 2, w / 2, -h / 2, h / 2);
	glMatrixMode(GL_MODELVIEW);
	

	// Update OpenGL viewport and internal variables
	glViewport(0, 0, w, h);
	Win[0] = w;
	Win[1] = h;
}



// display callback
//
// This gets called by the event handler to draw
// the scene, so this is where you need to build
// your scene -- make your changes and additions here.
// All rendering happens in this function.  For this Assignment,
// updates to geometry should happen in the "animate" function.
void display(void)
{
	// glClearColor (red, green, blue, alpha)
	// Ignore the meaning of the 'alpha' value for now
	/*glClearColor(0.7f, 0.7f, 0.9f, 1.0f);

	// OK, now clear the screen with the background colour
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup the model-view transformation matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	double aspect = (double)viewport[2] / (double)viewport[3];
	gluPerspective(60, aspect, 1, 600);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -200);
	glTranslatef(camPosX, camPosY, camPosZ);
	
	//Applied to object which is currently camera	
	glRotatef(camAngleX, 1.0, 0.0, 0.0);
	glRotatef(camAngleY, 0.0, 1.0, 0.0);
	glRotatef(camAngleZ, 0.0, 0.0, 1.0);

	///////////////////////////////////////////////////////////
	// TODO:
	//   Modify this function draw the scene
	//   This should include function calls to pieces that
	//   apply the appropriate transformation matrice and
	//   render the individual object parts.
	///////////////////////////////////////////////////////////

	// Draw our hinged object
	const float BODY_WIDTH = 200.0f;
	const float BODY_LENGTH = 50.0f;
	const float ARM_LENGTH = 50.0f;
	const float ARM_WIDTH = 10.0f;

	// Push the current transformation matrix on the stack
	glPushMatrix();
	glDrawCube();
	glPopMatrix();


	/*
	// Test drawings please ignore
	// Draw the 'body'
	glPushMatrix();

	glColor3f(0.0, 0.0, 0.0);
	//glScalef(BODY_WIDTH, BODY_WIDTH, BODY_WIDTH);
	glutSolidCube(BODY_LENGTH);
	glTranslatef(-1.0*BODY_LENGTH, -1.0*BODY_LENGTH, -1.0*BODY_LENGTH);
	glScalef(BODY_LENGTH, BODY_LENGTH, BODY_LENGTH);
	//glTranslatef(1.0*BODY_LENGTH, 1.0*BODY_LENGTH, 1.0*BODY_LENGTH);
	glPopMatrix();


	glPushMatrix();
	glColor3f(0.0, 0.0, 1.0);
	//glScalef(BODY_WIDTH, BODY_WIDTH, BODY_WIDTH);
	glTranslatef(1.0*BODY_LENGTH, 1.0*BODY_LENGTH, 1.0*BODY_LENGTH);
	glutSolidCube(BODY_LENGTH);	
	//glScalef(BODY_LENGTH, BODY_LENGTH, BODY_LENGTH);
	//glTranslatef(1.0*BODY_LENGTH, 1.0*BODY_LENGTH, 1.0*BODY_LENGTH);
	glPopMatrix();


	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0);
	//glScalef(BODY_WIDTH, BODY_WIDTH, BODY_WIDTH);
	glTranslatef(-1.0*BODY_LENGTH, -1.0*BODY_LENGTH, -1.0*BODY_LENGTH);
	glutSolidCube(BODY_LENGTH);	
	//glScalef(BODY_LENGTH, BODY_LENGTH, BODY_LENGTH);
	//glTranslatef();
	glPopMatrix();
	*/


	
	// Execute any GL functions that are in the queue just to be safe
	glFlush();

	// Now, show the frame buffer that we just drew into.
	// (this prevents flickering).
	glutSwapBuffers();
}


// Draw a square of the specified size, centered at the current location
void drawSquare(float width)
{
	// Draw the square
	glBegin(GL_POLYGON);
	glVertex2d(-width / 2, -width / 2);
	glVertex2d(width / 2, -width / 2);
	glVertex2d(width / 2, width / 2);
	glVertex2d(-width / 2, width / 2);
	glEnd();
}



//======================================FROM Source 1
/**************************************************************************
* glDrawCube
*  - Description:
*      - Contains the 3 loops for generating the 27 individual cublets.
*  - Parameters:(None)
*  - Local Variables
*      - xMark = x coordinate; yMark = y coordinate; zMark = z coordinate
*  -Returns:(void)
**************************************************************************
*/
void glDrawCube()
{
	float xMark = -1.0f;
	float yMark = -1.0f;
	float zMark = 1.0f;

	for (int i = 0; i < 3; i++)
		//moves back one unit on z-axis
	{
		glPushMatrix();
		glRotatef(rotation[i], 0.0f, 0.0f, 1.0f);
		for (int j = 0; j < 3; j++)
			//moves up one unit on y-axis
		{
			for (int k = 0; k < 3; k++)
				//builds 3 cubes along x-axis
			{
				buildCubeSolid(xMark, yMark, zMark);
				buildCubeWire(xMark, yMark, zMark);
				xMark++;
			}
			xMark = -1.0f;
			yMark++;
		}
		
		glPopMatrix();
		yMark = -1.0f;
		zMark--;
	}
}

/************************************************************
* buildCubeSolid
*  - Description:
*      - Draws the cublet using the OpenGL method GL_QUADS.
*  - Parameters:
*      - x: x coordinate to start drawing from.
*      - y: y coordinate to start drawing from.
*      - z: z coordinate to start drawing from.
*  - Local Varibles:(none)
*  - Returns(void)
*************************************************************
*/
void buildCubeSolid(float x, float y, float z)
{
	//draws a cube based on the center given
	glBegin(GL_QUADS);
	//TOP - GREEN
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f((x + 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y + 0.5f), (z + 0.5f));
		glVertex3f((x + 0.5f), (y + 0.5f), (z + 0.5f));
	//BOTTOM - ORANGE
		glColor3f(1.0f, 0.5f, 0.0f);
		glVertex3f((x + 0.5f), (y - 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z + 0.5f));
		glVertex3f((x + 0.5f), (y - 0.5f), (z + 0.5f));
	//FRONT - RED
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f((x + 0.5f), (y + 0.5f), (z + 0.5f));
		glVertex3f((x - 0.5f), (y + 0.5f), (z + 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z + 0.5f));
		glVertex3f((x + 0.5f), (y - 0.5f), (z + 0.5f));
	//BACK - YELLOW
		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f((x + 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z - 0.5f));
		glVertex3f((x + 0.5f), (y - 0.5f), (z - 0.5f));
	//LEFT - BLUE
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f((x - 0.5f), (y + 0.5f), (z + 0.5f));
		glVertex3f((x - 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z + 0.5f));
	//RIGHT - VIOLET
		glColor3f(1.0f, 0.0f, 1.0f);
		glVertex3f((x + 0.5f), (y + 0.5f), (z + 0.5f));
		glVertex3f((x + 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x + 0.5f), (y - 0.5f), (z - 0.5f));
		glVertex3f((x + 0.5f), (y - 0.5f), (z + 0.5f));
	glEnd();
}

/*******************************************************************************
* buildCubeWire
*  - Description:
*      - Draws the outline of the cublet using the OpenGL method GL_LINE_LOOP.
*  - Parameters:
*      - x: x coordinate to start drawing from.
*      - y: y coordinate to start drawing from.
*      - z: z coordinate to start drawing from.
*  - Local Varibles:(none)
*  - Returns(void)
*******************************************************************************
*/
void buildCubeWire(float x, float y, float z)
{
	//draw black lines on all edges of the minicubes
	//TOP
	glLineWidth(10);
	glBegin(GL_LINE_LOOP);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f((x + 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y + 0.5f), (z + 0.5f));
		glVertex3f((x + 0.5f), (y + 0.5f), (z + 0.5f));
	glEnd();
	//BOTTOM
	glBegin(GL_LINE_LOOP);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f((x + 0.5f), (y - 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z + 0.5f));
		glVertex3f((x + 0.5f), (y - 0.5f), (z + 0.5f));
	glEnd();
	//FRONT
	glBegin(GL_LINE_LOOP);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f((x + 0.5f), (y + 0.5f), (z + 0.5f));
		glVertex3f((x - 0.5f), (y + 0.5f), (z + 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z + 0.5f));
		glVertex3f((x + 0.5f), (y - 0.5f), (z + 0.5f));
	glEnd();
	//BACK
	glBegin(GL_LINE_LOOP);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f((x + 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z - 0.5f));
		glVertex3f((x + 0.5f), (y - 0.5f), (z - 0.5f));
	glEnd();
	//LEFT
	glBegin(GL_LINE_LOOP);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f((x - 0.5f), (y + 0.5f), (z + 0.5f));
		glVertex3f((x - 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z - 0.5f));
		glVertex3f((x - 0.5f), (y - 0.5f), (z + 0.5f));
	glEnd();
	//RIGHT
	glBegin(GL_LINE_LOOP);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f((x + 0.5f), (y + 0.5f), (z + 0.5f));
		glVertex3f((x + 0.5f), (y + 0.5f), (z - 0.5f));
		glVertex3f((x + 0.5f), (y - 0.5f), (z - 0.5f));
		glVertex3f((x + 0.5f), (y - 0.5f), (z + 0.5f));
	glEnd();
}