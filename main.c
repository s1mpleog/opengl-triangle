#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define WIDTH 800
#define HEIGHT 800

struct Context {
  GLFWwindow *window;
  // unsigned int shader_program;
};

void free_memory(char *buffer, long size) {

  if (munmap(buffer, size + 1) == -1) {
    printf("MUNMAP FAILED");
    return;
  }

  printf("Memory freed successfully.\n");
}

char *read_shader_from_file(const char *path, char **file_content, long *size) {
  FILE *file = fopen(path, "r");

  if (file == NULL) {
    perror("failed to open file\n");
    return NULL;
  }

  fseek(file, 0L, SEEK_END);
  *size = ftell(file);
  rewind(file);

  *file_content = mmap(NULL, *size + 1, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (*file_content == NULL) {
    perror("ERROR: failed to allocate memory mmap error\n");
    fclose(file);
    return NULL;
  }

  printf("MMAP ALLOCATED MEMORY ADDRESS: %p\n", file_content);

  fread(*file_content, *size, 1, file);
  (*file_content)[*size] = '\0';

  fclose(file);
  return *file_content;
}

int check_shader_compile_status(unsigned int shader) {
  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    printf("ERROR:SHADER:COMPILATION:FAILED %s\n", infoLog);
    return -1;
  }
  return 0;
}

int compile_vertex_shader(unsigned int *vertex_shader, char **buffer,
                          long *size) {
  char *vertex_shader_source =
      read_shader_from_file("./vertex.glsl", buffer, size);

  if (vertex_shader_source == NULL) {
    perror("failed to read vertex shader file");
    return EXIT_FAILURE;
  }

  const char *vertex_shader_content = *buffer;

  *vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(*vertex_shader, 1, &vertex_shader_content, NULL);
  glCompileShader(*vertex_shader);

  if (check_shader_compile_status(*vertex_shader) < 0) {
    printf("failed to compile vertex shader\n");
    return EXIT_FAILURE;
  };

  printf("from_compile_vertex_shader: %s\n", vertex_shader_source);
  return *vertex_shader;
}

int compile_fragment_shader(unsigned int *fragment_shader, char **buffer,
                            long *size) {

  printf("buffer_in_fragment %p\n", buffer);
  char *fragment_shader_source =
      read_shader_from_file("./fragment.glsl", buffer, size);

  if (fragment_shader_source == NULL) {
    perror("failed to read fragment shader file");
    return EXIT_FAILURE;
  }

  const char *fragment_shader_content = *buffer;
  *fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(*fragment_shader, 1, &fragment_shader_content, NULL);
  glCompileShader(*fragment_shader);

  if (check_shader_compile_status(*fragment_shader) < 0) {
    printf("failed to compile fragment shader\n");
    return EXIT_FAILURE;
  };

  printf("content: %s\n", fragment_shader_content);
  return EXIT_SUCCESS;
}

int check_shader_program_status(unsigned int *program) {
  int success;
  char info_log[512];

  glGetProgramiv(*program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(*program, 512, NULL, info_log);
    printf("ERROR::SHADER::PROGRAM::LINKING_FAILED%s\n", info_log);
    return -1;
  }
  return 0;
}

unsigned int create_shader_program(unsigned int *program,
                                   unsigned int *vertex_shader,
                                   unsigned int *fragment_shader) {
  *program = glCreateProgram();
  glAttachShader(*program, *vertex_shader);
  glAttachShader(*program, *fragment_shader);
  glLinkProgram(*program);

  if (check_shader_program_status(program) < 0) {
    printf("failed to compile shader program\n");
    return -1;
  }

  return *program;
}

void frame_buffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

int main(int argc, const char *argv[]) {

  printf("opengl window\n");

  if (!glfwInit()) {
    printf("failed to init glfw\n");
    return -1;
  }

  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "my window", NULL, NULL);

  if (window == NULL) {
    printf("failed to create window\n");
    glfwTerminate();
    return -1;
  }

  struct Context *context = malloc(sizeof(struct Context));
  if (context == NULL) {
    fprintf(stderr, "Failed to allocate memory for context\n");
    glfwTerminate();
    return -1;
  }
  context->window = window;

  glfwMakeContextCurrent(window);
  glViewport(0, 0, WIDTH, HEIGHT);
  glfwSetFramebufferSizeCallback(window, frame_buffer_size_callback);

  if (glewInit() != GLEW_OK) {
    printf("failed to initialize glew: %s\n", glewGetErrorString(glewInit()));
    return -1;
  }

  char *buffer;
  long size;
  unsigned int vertex_shader;
  unsigned int fragment_shader;
  unsigned int shader_program;

  int vertex_shader_compile =
      compile_vertex_shader(&vertex_shader, &buffer, &size);

  free_memory(buffer, size);

  int fragment_shader_compile =
      compile_fragment_shader(&fragment_shader, &buffer, &size);

  free_memory(buffer, size);

  unsigned int created_shader_program =
      create_shader_program(&shader_program, &vertex_shader, &fragment_shader);

  if (created_shader_program < 0) {
    perror("FAILED_TO_LINK_SHADER_PROGRAM\n");
    return EXIT_FAILURE;
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};

  unsigned int VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2, 0.3, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  free(context);
  glfwTerminate();
  return 0;
}
