//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

//Screen parameters
int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

//Projection and Light
glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

glm::vec3 lightPosEye;
GLuint lightPosEyeLoc;

glm::vec3 dominantDirectionalLight;
GLuint dominantDirectionalLightLoc;

glm::vec2 lightIntensity;
GLuint lightIntensityLoc;

//Camera
gps::Camera myCamera(glm::vec3(0.0f, 0.0f, 2.5f), glm::vec3(0.0f, 0.0f, -10.0f));
float cameraSpeed = 0.01f;

bool pressedKeys[1024];
float angle = 0.0f;
float moonAngle = 0.0f;

//for mouse callback
GLfloat prevX = 0;
GLfloat prevY = 0;

//objects
gps::Model3D myModel;
gps::Model3D moon;
gps::Model3D ground;
gps::Model3D mountain;
gps::Model3D tree;
gps::Model3D serpertine_city;

//skyBox shader
gps::Shader myCustomShader;
gps::Shader skyboxShader;

//skybox
gps::SkyBox mySkyBox;
std::vector<const GLchar*> faces;

//movement and animation variables
float intensity = 1000;
bool resetCamera = false;
bool mouseMoved = false;
float PI = 3.14159265359;


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 10000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	float xOffset;
	float yOffset;

	if (!mouseMoved)
	{
		prevX = xpos;
		prevY = ypos;
		mouseMoved = true;
	}

	xOffset = xpos - prevX;
	yOffset = prevY - ypos;

	xOffset *= 0.1f;
	yOffset *= 0.1f;

	prevX = xpos;
	prevY = ypos;

	myCamera.rotate(yOffset, xOffset);
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_1])
	{
		cameraSpeed += 0.1f;
	}

	if (pressedKeys[GLFW_KEY_2])
	{
		cameraSpeed -= 0.1f;
		if (cameraSpeed < 0)
			cameraSpeed = 0;
	}

	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 10.0f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 10.0f;
		if (angle < 0.0f)
			angle += 360.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_M]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_N]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_I]) {
		intensity += 50.0f;
	}

	if (pressedKeys[GLFW_KEY_O]) {
		intensity -= 50.0f;
	}

	moonAngle += 0.001f;
	if (moonAngle > 360.0f) {
		moonAngle = 0.0f;
	}
		

}


bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//MSAA - Multi-Sample Anti-Aliasing
	//glfwWindowHint(GLFW_SAMPLES, 16);
	//glEnable(GL_MULTISAMPLE);

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "City", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initModels()
{
	moon = gps::Model3D("objects/moon/moon.obj", "objects/moon/");
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	serpertine_city = gps::Model3D("objects/serpertine_city/serpertine_city.obj", "objects/serpertine_city/");
	mountain = gps::Model3D("objects/mountain/mountain.obj", "objects/mountain/");
	tree = gps::Model3D("objects/tree/tree.obj", "objects/tree/");
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
}

void initUniforms()
{
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 100000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//set Dominant Directional Light position
	dominantDirectionalLight = glm::vec3(1, -1, 0);
	dominantDirectionalLightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "dominantDirectionalLight");
	glUniform3fv(dominantDirectionalLightLoc, 1, glm::value_ptr(dominantDirectionalLight));
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.4, 0.9, 1, 1.0);
	//glViewport(0, 0, retina_width, retina_height);

	processMovement();

	//initialize the view matrix
	view = myCamera.getViewMatrix();
	//send view matrix data to shader	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));;

	//CITY
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(200, 0, -100));
	//	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	//model = glm::rotate(model, float(PI / 2), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	serpertine_city.Draw(myCustomShader);

	//CITY 2
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-200, 0, 0));
	//	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	model = glm::rotate(model, float(PI / 2), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	serpertine_city.Draw(myCustomShader);

	//CITY 3
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0, 0, -300));
	//	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	model = glm::rotate(model, float(PI / 3), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	serpertine_city.Draw(myCustomShader);

	//CITY 4
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0, 0, 200));
	//	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	model = glm::rotate(model, float(PI / 4), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	serpertine_city.Draw(myCustomShader);

	//TREE
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-200, 0, 200));
	model = glm::scale(model, glm::vec3(80, 70, 80));
	model = glm::rotate(model, float(PI / 4), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	//TREE 2
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-200, 0, -350));
	model = glm::scale(model, glm::vec3(70, 70, 70));
	model = glm::rotate(model, float(PI / 4), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	//TREE 3
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(300, 0, -400));
	model = glm::scale(model, glm::vec3(120, 70, 120));
	model = glm::rotate(model, float(PI / 4), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	//TREE 4
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0, 0, -500));
	model = glm::scale(model, glm::vec3(70, 70, 70));
	model = glm::rotate(model, float(PI / 4), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	//TREE 5
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(600, 0, -500));
	model = glm::scale(model, glm::vec3(100, 60, 100));
	model = glm::rotate(model, float(PI / 4), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	//TREE 6
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(600, 0, 0));
	model = glm::scale(model, glm::vec3(90, 80, 90));
	model = glm::rotate(model, float(PI / 4), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	//GROUND
	model = glm::mat4(1.0f);
	for (int i = 0; i < 1800; i += 20)
	{
		for (int j = 0; j < 1800; j += 20)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(i, 0, j));
			model = glm::translate(model, glm::vec3(-40 * 15, -1, -40 * 15));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			ground.Draw(myCustomShader);
		}
	}

	//Mountains
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2200, -100, 0));
	model = glm::scale(model, glm::vec3(1, 2.5f, 1));
	model = glm::rotate(model, PI * 3 / 2, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	mountain.Draw(myCustomShader);

	//Mountains
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1800, -100, 0));
	model = glm::scale(model, glm::vec3(1, 2.5f, 1));
	model = glm::rotate(model, PI * 1 / 2, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	mountain.Draw(myCustomShader);

	//Mountains
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0, -100, -1850));
	model = glm::scale(model, glm::vec3(1, 2.5f, 1));
	model = glm::rotate(model, PI * 1 / 2, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	mountain.Draw(myCustomShader);

	//Mountains
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0, -100, 2100));
	model = glm::scale(model, glm::vec3(1, 2.5f, 1));
	model = glm::rotate(model, PI * 3 / 2, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	mountain.Draw(myCustomShader);

	///LIGHT
	//set Point Light position
	lightPosEye = glm::vec3(0 ,500, 0);
	lightPosEyeLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosEye");
	glUniform3fv(lightPosEyeLoc, 1, glm::value_ptr(lightPosEye));
	//set Point Light intensity
	lightIntensity = glm::vec2(intensity, 0);
	lightIntensityLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightIntensity");
	glUniform2fv(lightIntensityLoc, 1, glm::value_ptr(lightIntensity));
	
	//MOON
	model = glm::mat4(1.0f);
	model = glm::rotate(model, moonAngle, glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(-1500, 5000, -18050));
	model = glm::scale(model, glm::vec3(101, 101, 101));
	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	moon.Draw(myCustomShader);

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	mySkyBox.Draw(skyboxShader, view, projection);

	myCustomShader.useShaderProgram();

}

int main(int argc, const char * argv[]) {
	
	initOpenGLWindow();
	initOpenGLState();
	initModels();
	initShaders();
	initUniforms();	

	faces.push_back("objects/skybox/earth/SunSet/right.png");
	faces.push_back("objects/skybox/earth/SunSet/left.png");
	faces.push_back("objects/skybox/earth/SunSet/top.png");
	faces.push_back("objects/skybox/earth/SunSet/bottom.png");
	faces.push_back("objects/skybox/earth/SunSet/back.png");
	faces.push_back("objects/skybox/earth/SunSet/front.png");

	mySkyBox.Load(faces);

	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}

