/* Possible solution to lab 5 - Real Time 3D*/

// Windows specific: Uncomment the following line to open a console window for debug output
#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif

#include "rt3d.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stack>
#define DEG_TO_RADIAN 0.017453293

using namespace std;

// Globals
// Data would normally be read from files

GLuint cubeVertCount = 8;
GLfloat cubeVerts[] = { -0.5, -0.5f, -0.5f,
-0.5, 0.5f, -0.5f,
0.5, 0.5f, -0.5f,
0.5, -0.5f, -0.5f,
-0.5, -0.5f, 0.5f,
-0.5, 0.5f, 0.5f,
0.5, 0.5f, 0.5f,
0.5, -0.5f, 0.5f };

GLuint cubeIndexCount = 36;
GLuint cubeIndices[] = { 0,1,2, 0,2,3, // back 
1,0,5, 0,4,5, // left
6,3,2, 3,6,7, // right
1,5,6, 1,6,2, // top
0,3,4, 3,7,4, // bottom
6,5,4, 7,6,4 }; // front

rt3d::lightStruct light0 = {
	{0.2f, 0.2f, 0.2f, 1.0f}, // ambient
	{0.7f, 0.7f, 0.7f, 1.0f}, // diffuse
	{0.8f, 0.8f, 0.8f, 1.0f}, // specular
	{0.0f, 0.0f, 1.0f, 1.0f}  // position
};

rt3d::materialStruct material0 = {
	{0.4f, 0.2f, 0.2f, 1.0f}, // ambient
	{0.8f, 0.5f, 0.5f, 1.0f}, // diffuse
	{1.0f, 0.8f, 0.8f, 1.0f}, // specular
	2.0f  // shininess
};

//Object position
GLfloat dx = 0.0f;
GLfloat dy = 0.0f;
GLfloat s = 1.0f;
GLfloat theta = 0.0f;

//Light position
GLfloat dxl = 0.0f;
GLfloat dyl = 0.0f;
GLfloat sl = 1.0f;
GLfloat thetal = 0.0f;
GLuint mvpShaderProgram;
GLuint mvpShaderProgram2;

//glm::vec4 lightpos_ini;
//glm::mat4 MVP;
GLuint meshObjects[1];
bool light_mode = false;
stack<glm::mat4> mvStack;

// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
		rt3d::exitFatalError("Unable to initialize SDL");

	// Request an OpenGL 3.3 context.
	// If you request a context not supported by your drivers, no OpenGL context will be created

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)

	// Create 800x600 window
	window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!window) // Check window was created OK
		rt3d::exitFatalError("Unable to create window");

	context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
	SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

void init(void) {

	//light properties declared here if they are not going to change
	//If that's not the case, use globals :-(

	//mvpShaderProgram = rt3d::initShaders("gouraud.vert","simple.frag");
	mvpShaderProgram = rt3d::initShaders("phong.vert", "phong.frag");

	//Set light and material
	rt3d::setLight(mvpShaderProgram, light0);
	rt3d::setMaterial(mvpShaderProgram, material0);

	// Going to create our mesh objects here
	// Using cubeVerts as a normal. This is not accurate for a cube, but it will do. Alternatively, we can calculate them and provide them.
	meshObjects[0] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeVerts, nullptr, cubeIndexCount, cubeIndices);


	glEnable(GL_DEPTH_TEST); // enable depth testing

}

void draw(SDL_Window * window) {
	// clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//Enable depth checking

	glm::mat4 identity(1.0);
	glm::mat4 projection(1.0);
	glm::mat4 modelview(1.0);
	glm::mat4 lighttransform(1.0);
	glm::vec4 lightpos = glm::vec4(light0.position[0], light0.position[1], light0.position[2], light0.position[3]);//initialise light position;	

	projection = glm::perspective(GLfloat(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 50.0f);
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "projection", glm::value_ptr(projection));

	//Send light properties
	rt3d::setLight(mvpShaderProgram, light0);
	glm::vec3 pos;

	rt3d::materialStruct material[9];
	//int k=0;
	for (int i = -1; i <= 1; i += 1)
	{

		for (int j = -1; j <= 1; j += 1)
		{
			material[i + j + 2] = material0;
			// render first element
			pos = glm::vec3(i*2.0f + dx, j*2.0f + dy, -4.0f);
			//cout<<"("<<pos.x<<","<<pos.y<<","<<pos.z<<")"<<endl;
			mvStack.push(modelview); // push modelview to stack
			mvStack.top() = glm::translate(mvStack.top(), pos);
			mvStack.top() = glm::rotate(mvStack.top(), theta, glm::vec3(0.0f, 1.0f, 0.0f));
			mvStack.top() = glm::scale(mvStack.top(), glm::vec3(s, s, s));
			rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
			mvStack.pop();
			rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);//For indexed mesh

			//Set material properties for each of the cubes
			material[i + j + 2].ambient[j + 1] = float(0.3f*(i + 1));
			material[i + j + 2].diffuse[j + 1] = float(0.3f*(i + 1));
			material[i + j + 2].specular[j + 1] = float(0.3f*(i + 1));
			material[i + j + 2].shininess = float(2.0f + 10.0f*(i + 2 + j));
			rt3d::setMaterial(mvpShaderProgram, material[i + j + 2]);
			//k+=1;
		}
	}

	//Transformations to move the light	
	lighttransform = glm::translate(lighttransform, glm::vec3(dxl, dyl, 0));
	lighttransform = glm::scale(lighttransform, glm::vec3(sl, sl, sl));
	lighttransform = glm::rotate(lighttransform, thetal, glm::vec3(0.0f, 1.0f, 0.0f));
	lightpos = lighttransform * lightpos;//Light is positioned with respect to the object position, but rotation and scale do not affect it
	GLfloat lightpos2[4] = { lightpos[0],lightpos[1],lightpos[2],lightpos[3] };
	rt3d::setLightPos(mvpShaderProgram, lightpos2);

	SDL_GL_SwapWindow(window); // swap buffers
}

void update(void) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_L]) light_mode = !light_mode;//toggle lightmode
	if (light_mode)
	{
		if (keys[SDL_SCANCODE_W]) dyl += 0.01f;
		if (keys[SDL_SCANCODE_S]) dyl -= 0.01f;
		if (keys[SDL_SCANCODE_A]) dxl -= 0.01f;
		if (keys[SDL_SCANCODE_D]) dxl += 0.01f;
		if (keys[SDL_SCANCODE_DOWN]) sl -= 0.01f;
		if (keys[SDL_SCANCODE_UP]) sl += 0.01f;
		if (keys[SDL_SCANCODE_RIGHT]) thetal -= 5.0f*DEG_TO_RADIAN;
		if (keys[SDL_SCANCODE_LEFT]) thetal += 5.0f*DEG_TO_RADIAN;
	}
	else {
		if (keys[SDL_SCANCODE_W]) dy += 0.01f;
		if (keys[SDL_SCANCODE_S]) dy -= 0.01f;
		if (keys[SDL_SCANCODE_A]) dx -= 0.01f;
		if (keys[SDL_SCANCODE_D]) dx += 0.01f;
		if (keys[SDL_SCANCODE_DOWN]) s -= 0.01f;
		if (keys[SDL_SCANCODE_UP]) s += 0.01f;
		if (keys[SDL_SCANCODE_RIGHT]) theta -= 5.0f*DEG_TO_RADIAN;
		if (keys[SDL_SCANCODE_LEFT]) theta += 5.0f*DEG_TO_RADIAN;
	}

	if (keys[SDL_SCANCODE_0])
	{
		dy = 0.0f; dx = 0.0f; s = 1.0f; theta = 0.0f;
		dyl = 0.0f; dxl = 0.0f; sl = 1.0f; thetal = 0.0f;
	}

	///Managing light changes. Difficult since there are nine components to control
	if (keys[SDL_SCANCODE_R])
	{
		light0.ambient[0] < 1.0f ? light0.ambient[0] += 0.01f : light0.ambient[0] = 0.0f; //light increase for Red
	}
	if (keys[SDL_SCANCODE_G])
	{
		light0.ambient[1] < 1.0f ? light0.ambient[1] += 0.01f : light0.ambient[1] = 0.0f; //light increase for Green
	}//light increase for Green
	if (keys[SDL_SCANCODE_B]) {
		light0.ambient[2] < 1.0f ? light0.ambient[2] += 0.01f : light0.ambient[2] = 0.0f; //light increase for Blue
	}//light increase for Blue
	if (keys[SDL_SCANCODE_O]) //toggle shader program to Gouraud
	{
		mvpShaderProgram = rt3d::initShaders("gouraud.vert", "simple.frag");
		//Set light and material
		rt3d::setLight(mvpShaderProgram, light0);
		rt3d::setMaterial(mvpShaderProgram, material0);
	}
	if (keys[SDL_SCANCODE_P]) //toggle shader program to Phong
	{
		mvpShaderProgram = rt3d::initShaders("phong.vert", "phong.frag");
		//Set light and material
		rt3d::setLight(mvpShaderProgram, light0);
		rt3d::setMaterial(mvpShaderProgram, material0);
	}
}


// Program entry point - SDL manages the actual WinMain entry point for us
int main(int argc, char *argv[]) {
	SDL_Window * hWindow; // window handle
	SDL_GLContext glContext; // OpenGL context handle
	hWindow = setupRC(glContext); // Create window and render context 

	// Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit(1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

	bool running = true; // set running to true
	SDL_Event sdlEvent;  // variable to detect SDL events
	while (running) {	// the event loop
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT)
				running = false;
		}
		update();//call the update function to poll the keyboard
		draw(hWindow); // call the draw function
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(hWindow);
	SDL_Quit();
	return 0;
}