#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <fstream>
#include <string>
#include <iostream>
static unsigned int ss_id = 0;
void dump_framebuffer_to_ppm(std::string prefix, unsigned int width, unsigned int height) {
    int pixelChannel = 3;
    int totalPixelSize = pixelChannel * width * height * sizeof(GLubyte);
    GLubyte* pixels = new GLubyte[totalPixelSize];
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    std::string file_name = prefix + std::to_string(ss_id) + ".ppm";
    std::ofstream fout(file_name);
    fout << "P3\n" << width << " " << height << "\n" << 255 << std::endl;
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            size_t cur = pixelChannel * ((height - i - 1) * width + j);
            fout << (int)pixels[cur] << " " << (int)pixels[cur + 1] << " " << (int)pixels[cur + 2] << " ";
        }
        fout << std::endl;
    }
    ss_id++;
    delete[] pixels;
    fout.flush();
    fout.close();
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// planet positions
glm::vec3 earthPos(20.0f, 0.0f, 0.0f);
glm::vec3 moonPos(30.0f, 0.0f, 0.0f);

float get_sun_rotate_angle_around_itself(float day) {
    float angle = 360.0f / 27.0f * day;
    return glm::radians(angle);
}

float get_earth_rotate_angle_around_sun(float day) {
    float angle = 360.0f / 365.0f * day + 90.0f;
    return glm::radians(angle);
}

float get_earth_rotate_angle_around_itself(float day) {
    float angle = 360.0f * day;
    return glm::radians(angle);
}

float get_moon_rotate_angle_around_earth(float day) {
    float angle = 360.0f / 28.0f * day + 90.0f;
    return glm::radians(angle);
}

float get_moon_rotate_angle_around_itself(float day) {
    float angle = 360.0f / 28.0f * day;
    return glm::radians(angle);
}


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment1 - Andrew Vu 400186041", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader
    // model, view, projection, and vertices information is passed through the vertex shader to get position
    // rgb information is also passed through vertex shader
    // frag shader recieves vertex colour and sets fragment colour
    Shader lightingShader("colors.vs", "colors.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // format is (x,y,x,r,g,b) for each triangle (12 triangles in total to make cube)
    // ------------------------------------------------------------------
    float vertices[] = {
        -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,

        -1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 1.0f,

        -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,

         1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
         1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
         1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 1.0f, 1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 0.0f,

        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
         1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    };

    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, sunVAO;
    glGenVertexArrays(1, &sunVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(sunVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // earth's VAO (VBO not needed since vertices are the same)
    unsigned int earthVAO;
    glGenVertexArrays(1, &earthVAO);
    glBindVertexArray(earthVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // moon's VAO
    unsigned int moonVAO;
    glGenVertexArrays(1, &moonVAO);
    glBindVertexArray(moonVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    float day = 0.0f;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        day += 1.0f / 24; //dynamic day
        //day = 365.0f; //set constant day
        //gets current x and z position of earth relative to day
        float earth_x = glm::sin(get_earth_rotate_angle_around_sun(day)) * 20.0;
        float earth_z = glm::cos(get_earth_rotate_angle_around_sun(day)) * 20.0;
        //gets current x and z position of moon relative to earth
        float moon_x = earth_x + glm::sin(get_moon_rotate_angle_around_earth(day)) * 10.0;
        float moon_z = earth_z + glm::cos(get_moon_rotate_angle_around_earth(day)) * 10.0;

        // input
        // -----
        processInput(window);
        
        // render
        // ------
        glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // activate shader
        lightingShader.use();

        glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(4.0f)); //scale to 8
        model = glm::rotate(model, get_sun_rotate_angle_around_itself(day), glm::vec3(0.0f, 1.0f, 0.0f)); //rotate sun on y axis
        view = glm::lookAt(
            glm::vec3(50,50,100), //camera position
            glm::vec3(0,0,0), //coordinate where camera is looking at
            glm::vec3(0,1,0)
            );
        projection = glm::perspective(glm::radians(30.0f), 4.0f/3.0f, 0.1f, 1000.0f);
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("model", model);

        // draw sun
        glBindVertexArray(sunVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // draw earth
        lightingShader.use();

        model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3(earth_x, 0, earth_z)); //sets position of earth so it revolves around sun relative to day
        model = glm::rotate(model, get_earth_rotate_angle_around_itself(day), glm::vec3(glm::radians(23.4) , 1.0f, 0.0f)); //rotate earth at 23.4 deg to y axis
        model = glm::scale(model, glm::vec3(2.5f)); // scale to 5
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("model", model);

        glBindVertexArray(earthVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // draw moon
        lightingShader.use();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(moon_x, 0, moon_z)); //sets position of moon relative to earth
        model = glm::scale(model, glm::vec3(1.5f)); // scale to 3
        model = glm::rotate(model, get_moon_rotate_angle_around_itself(day), glm::vec3(0.0f, 1.0f, 0.0f)); //rotate moon on y axis

        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("model", model);

        glBindVertexArray(moonVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &sunVAO);
    glDeleteVertexArrays(1, &earthVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    //press p to capture screen
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        std::cout << "Capture Window " << ss_id << std::endl;
        int buffer_width, buffer_height;
        glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
        dump_framebuffer_to_ppm("Assignment0-ss", buffer_width, buffer_height);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
