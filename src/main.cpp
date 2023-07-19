#include <iostream>
#include <cmath>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <OpenSimplexNoise.h>

#include "Shader.hpp"
#include "Camera.hpp"
#include "CubeRenderer.hpp"
#include "Block.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void mouse_callback(GLFWwindow* window, int button, int action, int mods);

void updateCamera(GLFWwindow* window, Camera& camera, float moveSpeed, float deltaTime);

glm::vec4 mousePicker(GLFWwindow* window, Camera& camera);

int main()
{
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, 800, 600);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Shader shaderProgram("./shader.vs", "./shader.fs");
    Camera playerCamera;
    playerCamera.setPosition(glm::vec3(10, 10, 5));

    CubeRenderer cubeRenderer;
    cubeRenderer.initialiseBuffers();
    cubeRenderer.loadTextures();

    srand(time(0));
    std::array<std::array<std::array<Block, 10>, 100>, 100> blocks;
    OpenSimplexNoise::Noise terrain_noise(rand());
    OpenSimplexNoise::Noise terrain_noise1(rand() * 2);
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            double height_value = terrain_noise.eval(i / 10, j / 10) * 3 + terrain_noise1.eval(i / 5, j / 5) * 4 + 3;
            for (int p = 0; p < 10; p++)
            {
                if (p < height_value)
                {
                    blocks[i][j][p].type = 1;
                }
                else
                {
                    blocks[i][j][p].type = 0;
                }
            }
        }
    }
    // testing visible faces
    /*
    for (int x = 0; x < 100; x++)
    {
        for (int z = 0; z < 100; z++)
        {
            for (int y = 0; y < 10; y++)
            {
                Block& block = blocks[x][z][y];
                if (x > 0)
                {
                    if (blocks[x - 1][z][y].type == 0)
                    {
                        //blocks[x][z][y].nXVisible = false;
                        block.face_indexes.push_back(10);
                        block.face_indexes.push_back(1);
                        block.face_indexes.push_back(17);
                        block.face_indexes.push_back(1);
                        block.face_indexes.push_back(17);
                        block.face_indexes.push_back(4);
                    }
                }
                if (x < 99)
                {
                    if (blocks[x + 1][z][y].type == 0)
                    {
                        //blocks[x][z][y].pXVisible = false;
                        block.face_indexes.push_back(3);
                        block.face_indexes.push_back(14);
                        block.face_indexes.push_back(8);
                        block.face_indexes.push_back(14);
                        block.face_indexes.push_back(9);
                        block.face_indexes.push_back(19);
                    }
                }
                if (z > 0)
                {
                    if (blocks[x][z - 1][y].type == 0)
                    {
                        //blocks[x][z][y].nZVisible = false;
                        block.face_indexes.push_back(13);
                        block.face_indexes.push_back(11);
                        block.face_indexes.push_back(18);
                        block.face_indexes.push_back(11);
                        block.face_indexes.push_back(18);
                        block.face_indexes.push_back(16);
                    }
                }
                if (z < 99)
                {
                    if (blocks[x][z + 1][y].type == 0)
                    {
                        //blocks[x][z][y].pZVisible = false;
                        block.face_indexes.push_back(0);
                        block.face_indexes.push_back(2);
                        block.face_indexes.push_back(5);
                        block.face_indexes.push_back(2);
                        block.face_indexes.push_back(5);
                        block.face_indexes.push_back(7);
                    }
                }
                if (y > 0)
                {
                    if (blocks[x][z][y - 1].type == 0)
                    {
                        //blocks[x][z][y].nYVisible = false;
                        block.face_indexes.push_back(21);
                        block.face_indexes.push_back(22);
                        block.face_indexes.push_back(12);
                        block.face_indexes.push_back(22);
                        block.face_indexes.push_back(12);
                        block.face_indexes.push_back(15);
                    }
                }
                if (y < 9)
                {
                    if (blocks[x][z][y + 1].type == 0)
                    {
                        //blocks[x][z][y].pYVisible = false;
                        block.face_indexes.push_back(6);
                        block.face_indexes.push_back(9);
                        block.face_indexes.push_back(23);
                        block.face_indexes.push_back(9);
                        block.face_indexes.push_back(23);
                        block.face_indexes.push_back(20);
                    }
                }
            }
        }
    }
    */

    const float MOVE_SPEED = 1.6f;

    float lastTime = glfwGetTime();

    float lastFps = 0;
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    while (!glfwWindowShouldClose(window))
    {

        //processInput(window);

        float deltaTime = glfwGetTime() - lastTime;
        lastTime = glfwGetTime();

        lastFps += deltaTime;
        if (lastFps >= 0.5)
        {
            lastFps = 0;
            std::cout << "FPS: " << 1 / deltaTime << "\n";
        }

        updateCamera(window, playerCamera, MOVE_SPEED, deltaTime);

        glClearColor(0.2, 0.6, 0.7, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram.use();

        glm::mat4 viewMatrix = playerCamera.calcViewMatrix();
        glm::mat4 projectionMatrix = playerCamera.calcProjectionMatrix(800, 600);

        shaderProgram.setMat4("view", viewMatrix);
        shaderProgram.setMat4("projection", projectionMatrix);

        //cubeRenderer.renderCube(shaderProgram, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), 0, glm::vec3(0.2, 0.2, 0.2));

        for (int x = 0; x < 100; x++)
        {
            for (int z = 0; z < 100; z++)
            {
                for (int y = 0; y < 10; y++)
                {
                    if (blocks[x][z][y].type == 1)
                    {
                        cubeRenderer.renderCube(shaderProgram, blocks[x][z][y], glm::vec3(x * 0.4, y * 0.4, z * 0.4), glm::vec3(0, 1, 0), 0, glm::vec3(0.2, 0.2, 0.2));
                    }
                }
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwTerminate();

    return 0;

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    int inputMode = glfwGetInputMode(window, GLFW_CURSOR);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if (inputMode == GLFW_CURSOR_DISABLED)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetWindowShouldClose(window, true);
        }
    }
    
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{

    int inputMode = glfwGetInputMode(window, GLFW_CURSOR);

    if (inputMode == GLFW_CURSOR_NORMAL)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPos(window, 800 / 2, 600 / 2);
        }
    }

}

void updateCamera(GLFWwindow* window, Camera& camera, float moveSpeed, float deltaTime)
{

    if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
    {
        return;
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    glm::vec2 mouse_rotation;
    mouse_rotation.y = static_cast<float>(ypos - (600 / 2)) / 600;
    mouse_rotation.x = static_cast<float>(xpos - (800 / 2)) / 800;

    glfwSetCursorPos(window, 800 / 2, 600 / 2);
    
    camera.updateDirection(mouse_rotation);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.move(CameraMoveDir::Forward, moveSpeed, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.move(CameraMoveDir::Back, moveSpeed, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.move(CameraMoveDir::Left, moveSpeed, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.move(CameraMoveDir::Right, moveSpeed, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera.move(CameraMoveDir::Up, moveSpeed, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        camera.move(CameraMoveDir::Down, moveSpeed, deltaTime);
    }

}

glm::vec4 mousePicker(GLFWwindow* window, Camera& camera)
{

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    float x = (2 * mouseX) / 800 - 1;
    float y = (2 * mouseY) / 600 - 1;
    glm::vec2 normalizedCoords = glm::vec2(x, -y);
    glm::mat4 inverseProjection = glm::inverse(camera.calcProjectionMatrix(800, 600));
    //glm::vec4 eyeCoords = normalizedCoords * inverseProjection;
    

}
