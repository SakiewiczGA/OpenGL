#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  //image loading utility funcitns

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h" //added as header and added code for UP and DOWN

using namespace std;


//shader program
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Gayle's Project";

    //Variables for window 
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    //Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;     // Handles for the vertex buffer objects
        GLuint nVertices;    // Number of indices of the mesh
    };


    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    GLuint planeVBO, planeVAO;
    GLuint surfaceVBO, surfaceVAO;
    GLuint lightVAO;
    GLuint lightVBO;
    GLuint pyrVAO;
    GLuint pyrVBO;
    GLuint cylVAO;
    GLuint cylVBO;
    bool isOrtho;
    bool gRainbow = true;

    //Texture
    GLuint gTextureId; //greenyellow crayon box
    GLuint gTextureIdRainbow;  //crayonbox top
    GLuint gTextureIdRed; //surface plane
    GLuint gTextureIdPyramid;
    GLuint gTextureIdBlue;

    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTextWrapMode = GL_REPEAT;

    // Shader programs
    GLuint gProgramId;
    GLuint gLampProgramId;

    //object and light color
    glm::vec3 gObjectColor(1.0f, 0.2f, 0.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gLightColor2(1.0f, 1.0f, 1.0f);



    // Light position and scale
    glm::vec3 gLightPosition(0.0f, 1.5f, 0.0f);
    glm::vec3 gLightPosition2(-1.5f, 2.0f, -0.5f);
    glm::vec3 gLightScale(0.3f);
    glm::vec3 gLightScale2(0.2f);
    glm::vec3 gObjectPostition(0.0f, 0.0f, 0.0f);
    glm::vec3 gObjectScale(2.0f);
    glm::vec3 gPyramidPosition(-1.0f, 0.0f, -0.5f);
    glm::vec3 gPyramidScale(2.0f);
    glm::vec3 gCylinderPosition(0.5f, 0.6f, 0.0f);
    glm::vec3 gCylinderScale(0.1f);


    //camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    //timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

}

/* User-defined Function prototypes to initialize the program, set window size,redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


//Vertex Shader Source Code
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec3 normal;  // Color data from Vertex Attrib Pointer 1
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // variable to transfer color data to the fragment shader
out vec3 vertexFragmentPos;
out vec2 vertexTextureCoordinate;
//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexFragmentPos = vec3(model * vec4(position, 1.0));
    vertexNormal = mat3(transpose(inverse(model))) * normal;
    vertexTextureCoordinate = textureCoordinate; //references incoming texture
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec3 vertexNormal;
in vec3 vertexFragmentPos;
in vec2 vertexTextureCoordinate; // Variable to hold incoming texture data from vertex shader

out vec4 fragmentColor;
// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightColor2;
uniform vec3 lightPos;
uniform vec3 lightPos2;

uniform vec3 viewPosition;
uniform sampler2D uTexture;
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

//Calculate Ambient lighting*/
    float ambientStrength = 0.7f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    //light2... ambient
    float ambentStrength2 = 0.5f;
    vec3 ambient2 = ambentStrength2 * lightColor2;
    //diffuse light2
    vec3 lightDirection2 = normalize(lightPos2 - vertexFragmentPos);
    float impact2 = max(dot(norm, lightDirection2), 0.0);
    vec3 diffuse2 = impact2 * lightColor2;
    //light2 specular
    float specularIntensity2 = 0.25f; // Set specular light strength
    float highlightSize2 = 32.0f; // Set specular highlight size

    vec3 reflectDir2 = reflect(-lightDirection2, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
    vec3 specular2 = specularIntensity2 * specularComponent2 * lightColor2;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate);

    // Calculate phong result
    vec3 light1Result = (ambient + diffuse + specular);
    vec3 light2Result = (ambient2 + diffuse2 + specular2);
    vec3 lightResult = light1Result + light2Result;
    vec3 phong = (lightResult)*textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU


}
);
/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Lamp Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}



// main- entry to the OpenGL program
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    //Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the VBO


    //Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;



    // Load texture
    const char* texFilename = "YellowGreen2.gif";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "pencils2.gif";
    if (!UCreateTexture(texFilename, gTextureIdRainbow))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "Red_Color.gif";
    if (!UCreateTexture(texFilename, gTextureIdRed))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "pyramid.gif";
    if (!UCreateTexture(texFilename, gTextureIdPyramid))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "blue2.gif";
    if (!UCreateTexture(texFilename, gTextureIdBlue))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    //  set the textures as texture units 0 and 1
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);


    // Set background of window 
    glClearColor(0.5f, 0.5f, 0.5f, 0.6f);


    // render loop

    while (!glfwWindowShouldClose(gWindow))
    {

        // per-frame timing
// --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh and other data
    UDestroyMesh(gMesh);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &surfaceVBO);
    glDeleteVertexArrays(1, &surfaceVAO);
    glDeleteBuffers(1, &pyrVBO);
    glDeleteVertexArrays(1, &pyrVAO);
    glDeleteVertexArrays(1, &cylVAO);
    glDeleteBuffers(1, &cylVBO);
    // Release texture
    UDestroyTexture(gTextureId);
    UDestroyTexture(gTextureIdRainbow);
    UDestroyTexture(gTextureIdRed);
    UDestroyTexture(gTextureIdPyramid);
    UDestroyTexture(gTextureIdBlue);

    // Release shader program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); //Ends the program successfully
}


// Initialize GLFW, GLEW, and create window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // Initialize GLFW with openGL version 4

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // create window

    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //Initialize GLEW


    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


//process all input: ask GLFW if relevent keys are pressed this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;
    isOrtho = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);



}


// if window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}




// Functioned called to render a frame
void URender()
{

    //enable z-depth
    glEnable(GL_DEPTH_TEST);

    //clear the frame and z buffers
    glClearColor(0.5f, 0.5f, 0.5f, 0.6f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 trans = glm::mat4(1.0f); //need this for scale, rotate, translate before vec3 argument
    // 1. Scales the object
    glm::mat4 scale = glm::scale(trans, glm::vec3(0.5f, 0.5f, 0.5f));//(1.0f, 0.5f, 2.0f));
    // 2. Rotates shape by 45 degrees in the x axis //changed to -55 1 0 0 from 0 1 1 1 
    glm::mat4 rotation = glm::rotate(trans, 45.0f, glm::vec3(0.0f, 0.0f, 1.0f));//0.0f, glm::vec3(1.0, 0.0f, 0.0f)); //-55.0
    // 3. Place object at the origin
    glm::mat4 translation = glm::translate(trans, glm::vec3(0.0f, 0.0f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();


    // Set the shader
    glUseProgram(gProgramId);
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);


    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //UV scale //tutorial line 441
    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Reference matrix uniforms from the object Shader program for the object color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint lightColorLoc2 = glGetUniformLocation(gProgramId, "lightColor2");
    GLint lightPositionLoc2 = glGetUniformLocation(gProgramId, "lightPos2");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(lightColorLoc2, gLightColor2.r, gLightColor2.g, gLightColor2.b);
    glUniform3f(lightPositionLoc2, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);


    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdRainbow);

    // Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    //deactivate texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    // Deactivate VAO
    glBindVertexArray(0);

    //plane for crayon box top
    GLfloat vertices[] = {
        //vertex                normals              texture                   
       -0.5f, -0.5f, -0.8f,    0.0f, 0.0f, -1.0f,    0.0f, 0.0f,           //v0 back left
        0.5f, -0.5f, -0.8f,    0.0f, 0.0f, -1.0f,     1.0f, 0.0f,          //v1 back right
        0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,     1.0f, 1.0f,          //v2front right
        0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,     1.0f, 1.0f,           //v2front right
       -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,     0.0f, 1.0f,            //v3 front left
       -0.5f, -0.5f, -0.8f,    0.0f, 0.0f, -1.0f,     0.0f, 0.0f            //v0 back left


    };



    glGenVertexArrays(1, &planeVAO);
    glBindVertexArray(planeVAO);

    glGenBuffers(1, &planeVBO);


    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes

    GLuint planeVertices = sizeof(vertices) / (sizeof(vertices[0] * (5)));

    GLint stride = sizeof(float) * (8);

    // Specify attribute location and layout to GPU
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (6)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(planeVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    glDrawArrays(GL_TRIANGLES, 0, planeVertices);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    //Surface plane
    GLfloat vert[] = {
        //vertex                  normals                //texture coords
       -5.0f,  -0.51f, -5.0f,     0.0f, 0.0f, -1.0f,   0.0f, 0.0f,  //v0 back left
        5.0f,  -0.51f, -5.0f,    0.0f, 0.0f, -1.0f,     1.0f, 0.0f,  //v1 back right
        5.0f,  -0.51f,  5.0f,    0.0f, 0.0f, -1.0f,     1.0f, 1.0f, //v2front right
        5.0f,  -0.51f,  5.0f,    0.0f, 0.0f, -1.0f,     1.0f, 1.0f, //v2front right
       -5.0f,  -0.51f,  5.0f,    0.0f, 0.0f, -1.0f,     0.0f, 1.0f, //v3 front left
       -5.0f,  -0.51f, -5.0f,    0.0f, 0.0f, -1.0f,     0.0f, 0.0f,  //v0 back left

    };



    glGenVertexArrays(1, &surfaceVAO);
    glBindVertexArray(surfaceVAO);

    glGenBuffers(1, &surfaceVBO);


    glBindBuffer(GL_ARRAY_BUFFER, surfaceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW); // Load vertex attributes

    GLuint surfaceVertices = sizeof(vert) / (sizeof(vert[0]) * (8));
    GLint stride2 = sizeof(float) * (8);


    // Specify attribute location and layout to GPU
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride2, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride2, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride2, (char*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);
    glBindVertexArray(surfaceVAO);

    //glTranslatef(0.0f, 5.0f, 0.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdRed);

    glDrawArrays(GL_TRIANGLES, 0, surfaceVertices);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(0);

    //
    //model = glm::translate(gPyramidPosition) * glm::scale(gPyramidScale);

    //pyramid
    GLfloat pVerts[] = {
        //triangle 1 front    //normals           //texture
        -1.5f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,     0.5f, 1.0f,  // top point
       -2.0f, -0.5f, 0.5f,   0.0f, 0.0f, 1.0f,     0.0f, 0.0f, //  left point
        -1.0f, -0.5f, 0.5f,   0.0f, 0.0f, 1.0f,    1.0f, 0.0f,//   right point

        //triangle 2  right
        -1.5f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,     0.5f, 1.0f,// top pt
        -1.0f, -0.5f, 0.5f,    1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // front right
        -1.0f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // back right

        //triangle 3  back
        -1.5f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,      0.5f, 1.0f,//  top pt
       -2.0f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,    0.0f, 0.0f,// back left
        -1.0f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,//  back rt

        //triangle 4  left
        -1.5f, 1.0f, 0.0f,    0.0f, 0.0f, 1.0f,    0.5f, 1.0f,// top pt
       -2.0f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,// back left
       -2.0f, -0.5f, 0.5f,    -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // front left

        //base
       -2.0f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,  0.0f, 0.0f,//   back left
       -1.0f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,  1.0f, 0.0f, //    back right
        -2.0f, -0.5f, 0.5f,   0.0f, -1.0f, 0.0f,  0.0f, 1.0f, //front left


       -2.0f, -0.5f, 0.5f,      0.0f, -1.0f, 0.0f,   0.0f, 1.0f, //front left  
       -1.0f, -0.5f, -0.5f,    0.0f, -1.0f, 0.0f,    1.0f, 0.0f, //back right
       -1.0f, -0.5f, 0.5f,     0.0f, -1.0f, 0.0f,    1.0f, 1.0f //front right

    };

    GLuint nPyrVerts = sizeof(pVerts) / (sizeof(pVerts[0]) * (8));
    glGenVertexArrays(1, &pyrVAO); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(pyrVAO);

    // Create buffer
    glGenBuffers(1, &pyrVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pyrVBO); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(pVerts), pVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint pStride = sizeof(float) * (8);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, pStride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (6)));
    glEnableVertexAttribArray(2);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdPyramid);



    glDrawArrays(GL_TRIANGLES, 0, nPyrVerts);


    //deactivate texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glBindVertexArray(0);

    //Transform cube used as a visual que for the light source
    //rotation = glm::rotate(trans, 45.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(gCylinderPosition) * rotation * glm::scale(gCylinderScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    float cyl[30 * 16];
    float o = 20.0f;
    float o2 = 0.0f;
    float texc[5 * 2 * 3 * 2];
    for (int i = 0; i < 16; i++) {
        o2 += (glm::pi<float>() * 2) / 16;
        cyl[i * 30] = sin(o);
        cyl[i * 30 + 1] = -1.f;
        cyl[i * 30 + 2] = cos(o);
        cyl[i * 30 + 3] = i * 1.0 / 16.0;
        cyl[i * 30 + 4] = 0.0;
        cyl[i * 30 + 5] = sin(o2);
        cyl[i * 30 + 6] = -1.f;
        cyl[i * 30 + 7] = cos(o2);
        cyl[i * 30 + 8] = (i + 1) * 1.0 / 16.0;
        cyl[i * 30 + 9] = 0.0;
        cyl[i * 30 + 10] = sin(o);
        cyl[i * 30 + 11] = 1.f;
        cyl[i * 30 + 12] = cos(o);
        cyl[i * 30 + 13] = i * 1.0 / 16.0;
        cyl[i * 30 + 14] = 1.0;
        cyl[i * 30 + 15] = sin(o2);
        cyl[i * 30 + 16] = 1.f;
        cyl[i * 30 + 17] = cos(o2);
        cyl[i * 30 + 18] = (i + 1) * 1.0 / 16.0;
        cyl[i * 30 + 19] = 1.0;
        cyl[i * 30 + 20] = sin(o);
        cyl[i * 30 + 21] = 1.f;
        cyl[i * 30 + 22] = cos(o);
        cyl[i * 30 + 23] = i * 1.0 / 16.0;
        cyl[i * 30 + 24] = 1.0;
        cyl[i * 30 + 25] = sin(o2);
        cyl[i * 30 + 26] = -1.f;
        cyl[i * 30 + 27] = cos(o2);
        cyl[i * 30 + 28] = (i + 1) * 1.0 / 16.0;
        cyl[i * 30 + 29] = 0.0;

        o += (glm::pi<float>() * 2) / 16;
    }

    glGenVertexArrays(1, &cylVAO);
    glGenBuffers(1, &cylVBO);

    glBindVertexArray(cylVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cylVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cyl), cyl, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBlue);

    glDrawArrays(GL_TRIANGLES, 0, 6 * 18);

    //deactivate texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glBindVertexArray(0);

    glUseProgram(gLampProgramId);

    //Transform cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLfloat verts[] = {

        //Back Face          //Negative Z Normal  Texture Coords.
          -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
           0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
           0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
           0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
          -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
          -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

          //Front Face         //Positive Z Normal
         -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

         //Left Face          //Negative X Normal
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        //Right Face         //Positive X Normal
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        //Bottom Face        //Negative Y Normal
       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
       -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

       //Top Face           //Positive Y Normal
      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
       0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    GLuint nverts = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &lightVAO); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(lightVAO);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU


    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);


    glDrawArrays(GL_TRIANGLES, 0, nverts);

    //2nd light
    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition2) * glm::scale(gLightScale2);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, nverts);



    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);



    // swap buffers and poll IO events,flips the the back buffer with the front buffer every frame
    glfwSwapBuffers(gWindow);



}



// Implement the create mesh function
void UCreateMesh(GLMesh& mesh)
{
    // Specify normalized device coordinates (x,y,z) and color (r,g,b,a) for triangle vertices
    GLfloat verts[] =
    {


        // back face               Normals             Texture coords
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,     0.0f, 0.0f,   //1
         0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,     1.0f, 0.0f,   //2
         0.5f,  0.0f, -0.5f,    0.0f, 0.0f, -1.0f,     1.0f, 1.0f,    //3
         0.5f,  0.0f, -0.5f,    0.0f, 0.0f, -1.0f,     1.0f, 1.0f,    //3
        -0.5f,  0.0f, -0.5f,    0.0f, 0.0f, -1.0f,     0.0f, 1.0f,    //4
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,     0.0f, 0.0f,  //1
        //front face          //normals                
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,     0.0f, 0.0f,   //5
         0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,     1.0f, 0.0f,  //6
         0.5f,  0.0f,  0.5f,   0.0f, 0.0f, 1.0f,     1.0f, 1.0f,   //7
         0.5f,  0.0f,  0.5f,   0.0f, 0.0f, 1.0f,     1.0f, 1.0f,   //7
        -0.5f,  0.0f,  0.5f,   0.0f, 0.0f, 1.0f,     0.0f, 1.0f,   //8
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,     0.0f, 0.0f,   //5   
        //left
        -0.5f,  0.0f,  0.5f,   -1.0f, 0.0f, 0.0f,    1.0f, 0.0f,  //8
        -0.5f,  0.0f, -0.5f,   -1.0f, 0.0f, 0.0f,    1.0f, 1.0f,   //4
        -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,    0.0f, 1.0f,   //1
        -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,    0.0f, 1.0f,   //1
        -0.5f, -0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,    0.0f, 0.0f,   //5
        -0.5f,  0.0f,  0.5f,   -1.0f, 0.0f, 0.0f,    1.0f, 0.0f,   //8
        //right
         0.5f,  0.0f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,   //7
         0.5f,  0.0f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,   //3
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   //2
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   //2
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,   //6
         0.5f,  0.0f,  0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   //7
         //bottom
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,   //1
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,   //2
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,   //6
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,   //6
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,    //5
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,  //1
        //top
        -0.5f,  0.0f, -0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f,   //4
         0.5f,  0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,  //3
         0.5f,  0.0f,  0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,   //7
         0.5f,  0.0f,  0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,   //7
        -0.5f,  0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,   //8
        -0.5f,  0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f   //4
    };



    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); //can generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: 1st one for the vertex data 2nd for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create a buffer object for the indices

    mesh.nVertices = sizeof(verts) / sizeof(verts[0]) * (floatsPerVertex + floatsPerUV + floatsPerNormal);


    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV + floatsPerNormal);// The number of floats before each


    // Creates the Vertex Attribute Pointer
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }
    // Error loading the image
    return false;
}

void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implement UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a shader program object
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrieve shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader and print compilation errors if any
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attach compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    //link shader program
    glLinkProgram(programId);
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);  // uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}

