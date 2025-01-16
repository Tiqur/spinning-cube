#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using std::cout, std::endl;
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  (void)window;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

const char *vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 transform;
    void main() {
        gl_Position = transform * vec4(aPos, 1.0);
    }
)";

const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
)";

class Shader {
public:
  Shader(const char *shaderSource, GLenum shaderType) {
    cout << "Creating shader..." << endl;
    m_id = glCreateShader(shaderType);
    glShaderSource(m_id, 1, &shaderSource, NULL);
    glCompileShader(m_id);

    int success;
    char infoLog[512];
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(m_id, 512, NULL, infoLog);
      cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }
  }

  ~Shader() {
    cout << "Deleting shader..." << endl;
    if (m_id != 0) {
      glDeleteShader(m_id);
    }
  }

  GLuint id() const { return m_id; }

private:
  GLuint m_id{};
};

class Program {
public:
  Program(Shader vertexShader, Shader fragmentShader) {
    cout << "Creating program..." << endl;
    m_id = glCreateProgram();
    glAttachShader(m_id, vertexShader.id());
    glAttachShader(m_id, fragmentShader.id());
    glLinkProgram(m_id);

    GLint success;
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[512];
      glGetProgramInfoLog(m_id, 512, NULL, infoLog);
      cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }
  }

  ~Program() {
    if (m_id != 0) {
      cout << "Deleting program..." << endl;
      glDeleteProgram(m_id);
    }
  }

  GLuint id() const { return m_id; }

private:
  GLuint m_id{};
};

class Buffer {
public:
  Buffer(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
    cout << "Creating buffer..." << endl;
    glGenBuffers(1, &m_id);
    glBindBuffer(target, m_id);
    glBufferData(target, size, data, usage);
  }

  ~Buffer() {
    if (m_id != 0) {
      cout << "Deleting buffer..." << endl;
      glDeleteBuffers(1, &m_id);
    }
  }

  GLuint id() const { return m_id; }

private:
  GLuint m_id{};
};

class VertexArray {
public:
  VertexArray() {
    cout << "Creating vertex array..." << endl;
    glGenVertexArrays(1, &m_id);
  }

  void bind() { glBindVertexArray(m_id); }

  ~VertexArray() {
    if (m_id != 0) {
      cout << "Deleting array..." << endl;
      glDeleteVertexArrays(1, &m_id);
    }
  }

  GLuint id() const { return m_id; }

private:
  GLuint m_id{};
};

float vertices[] = {
    0.5f,  0.5f,  -0.5f, //
    0.5f,  -0.5f, -0.5f, //
    -0.5f, -0.5f, -0.5f, //
    -0.5f, 0.5f,  -0.5f, //
    0.5f,  0.5f,  0.5f,  //
    0.5f,  -0.5f, 0.5f,  //
    -0.5f, -0.5f, 0.5f,  //
    -0.5f, 0.5f,  0.5f   //
};

unsigned int indices[] = {
    3, 0, 1, //
    3, 2, 1, //
    2, 1, 5, //
    2, 6, 5, //
    1, 0, 4, //
    1, 5, 4, //
    3, 2, 6, //
    3, 7, 6, //
    6, 5, 4, //
    7, 6, 4, //
    7, 4, 0, //
    7, 3, 0, //
};

int main() {
  // Initialize ImGui
  std::cout << "Initializing ImGui..." << std::endl;
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  // Make the OpenGL context current
  glfwMakeContextCurrent(window);

  std::cout << "Initializing ImGui GLFW backend..." << std::endl;
  if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
    std::cerr << "Failed to initialize ImGui GLFW backend!" << std::endl;
    return -1;
  }

  std::cout << "Initializing ImGui OpenGL backend..." << std::endl;
  if (!ImGui_ImplOpenGL3_Init("#version 330")) {
    std::cerr << "Failed to initialize ImGui OpenGL backend!" << std::endl;
    return -1;
  }

  // Initialize GLEW
  glewExperimental = GL_TRUE; // Ensure GLEW uses modern OpenGL techniques
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return -1;
  }

  // Set the viewport
  glViewport(0, 0, 800, 600);

  // Register the framebuffer size callback
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  Buffer VBO(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  Buffer EBO(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  Shader vertexShader(vertexShaderSource, GL_VERTEX_SHADER);
  Shader fragmentShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
  Program shaderProgram(vertexShader, fragmentShader);

  VertexArray VAO;
  VAO.bind();

  glBindBuffer(GL_ARRAY_BUFFER, VBO.id());
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Main render loop
  while (!glfwWindowShouldClose(window)) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // ImGui::ShowDemoWindow();

    static float angle = 0.0f;
    static float rotationDegrees = 0.005f;
    angle += rotationDegrees;

    bool active = true;
    float min = 0.0;
    float max = 0.1;
    ImGui::Begin("Settings", &active, ImGuiWindowFlags_MenuBar);
    ImGui::SliderScalar("Rotation angle", ImGuiDataType_Float, &rotationDegrees,
                        &min, &max, "%.2f degrees per frame");
    ImGui::End();

    // Render OpenGL
    glClearColor(0.2f, 0.4f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram.id());
    glBindVertexArray(VAO.id());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO.id());
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(float) * 3,
                   GL_UNSIGNED_INT, 0);

    glm::mat4 rotationMatrix =
        glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 1.0f, 1.0f));
    GLint transformLoc = glGetUniformLocation(shaderProgram.id(), "transform");
    glUseProgram(shaderProgram.id());
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE,
                       glm::value_ptr(rotationMatrix));

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Process user input
    processInput(window);

    // Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Unbind OpenGL objects
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);

  // Clean up and exit
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
