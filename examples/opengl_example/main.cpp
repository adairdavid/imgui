#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"                  // for .png loading
#include "../../imgui.h"

// glew & glfw
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <iostream>

static SDL_Window* window;
static SDL_GLContext context;
static GLuint fontTex;
static bool mousePressed[4] = { false, false };
static ImVec2 mousePosScale(1.0f, 1.0f);

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
// - try adjusting ImGui::GetIO().PixelCenterOffset to 0.5f or 0.375f
static void ImImpl_RenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count)
{
  if (cmd_lists_count == 0)
    return;

  // We are using the OpenGL fixed pipeline to make the example code simpler to read!
  // A probable faster way to render would be to collate all vertices from all cmd_lists into a single vertex buffer.
  // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  // Setup texture
  glBindTexture(GL_TEXTURE_2D, fontTex);
  glEnable(GL_TEXTURE_2D);

  // Setup orthographic projection matrix
  const float width = ImGui::GetIO().DisplaySize.x;
  const float height = ImGui::GetIO().DisplaySize.y;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Render command lists
  for (int n = 0; n < cmd_lists_count; n++)
  {
    const ImDrawList* cmd_list = cmd_lists[n];
    const unsigned char* vtx_buffer = (const unsigned char*)cmd_list->vtx_buffer.begin();
    glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer));
    glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer+8));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void*)(vtx_buffer+16));

    int vtx_offset = 0;
    const ImDrawCmd* pcmd_end = cmd_list->commands.end();
    for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++)
    {
      glScissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w), (int)(pcmd->clip_rect.z - pcmd->clip_rect.x), (int)(pcmd->clip_rect.w - pcmd->clip_rect.y));
      glDrawArrays(GL_TRIANGLES, vtx_offset, pcmd->vtx_count);
      vtx_offset += pcmd->vtx_count;
    }
  }
  glDisable(GL_SCISSOR_TEST);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void InitGL()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
      SDL_GL_CONTEXT_PROFILE_CORE);
  window = SDL_CreateWindow("SDL IMGui Example.",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
      SDL_WINDOW_OPENGL);
  context = SDL_GL_CreateContext(window);

  glewInit();
}

void InitImGui()
{
  int w, h;
  int fb_w, fb_h;
  SDL_GetWindowSize(window, &w, &h);
  SDL_GetWindowSize(window, &fb_w, &fb_h); // Needs to be corrected for SDL Framebuffer
  mousePosScale.x = (float)fb_w / w;
  mousePosScale.y = (float)fb_h / h;

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2((float)fb_w, (float)fb_h);  // Display size, in pixels. For clamping windows positions.
  io.PixelCenterOffset = 0.0f;                        // Align OpenGL texels

  io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;             // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
  io.KeyMap[ImGuiKey_LeftArrow] = SDLK_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = SDLK_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = SDLK_UP;
  io.KeyMap[ImGuiKey_DownArrow] = SDLK_DOWN;
  io.KeyMap[ImGuiKey_Home] = SDLK_HOME;
  io.KeyMap[ImGuiKey_End] = SDLK_END;
  io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
  io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
  io.KeyMap[ImGuiKey_A] = SDLK_a;
  io.KeyMap[ImGuiKey_C] = SDLK_c;
  io.KeyMap[ImGuiKey_V] = SDLK_v;
  io.KeyMap[ImGuiKey_X] = SDLK_x;
  io.KeyMap[ImGuiKey_Y] = SDLK_y;
  io.KeyMap[ImGuiKey_Z] = SDLK_z;

  io.RenderDrawListsFn = ImImpl_RenderDrawLists;

  // Load font texture
  glGenTextures(1, &fontTex);
  glBindTexture(GL_TEXTURE_2D, fontTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Default font (embedded in code)
  const void* png_data;
  unsigned int png_size;
  ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
  int tex_x, tex_y, tex_comp;
  void* tex_data = stbi_load_from_memory((const unsigned char*)png_data, (int)png_size, &tex_x, &tex_y, &tex_comp, 0);
  IM_ASSERT(tex_data != NULL);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_x, tex_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
  stbi_image_free(tex_data);
}

void UpdateImGui()
{
  ImGuiIO& io = ImGui::GetIO();

  // Setup inputs
  // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
  int mouse_x, mouse_y;
  SDL_GetMouseState(&mouse_x, &mouse_y);
  io.MousePos = ImVec2((float)mouse_x * mousePosScale.x, (float)mouse_y * mousePosScale.y);      // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)

  io.MouseDown[0] = mousePressed[0] || SDL_BUTTON(SDL_BUTTON_LEFT);
  io.MouseDown[1] = mousePressed[1] || SDL_BUTTON(SDL_BUTTON_RIGHT);

  // Start the frame
  ImGui::NewFrame();
}

// Application code
int main(int argc, char** argv)
{
  InitGL();
  InitImGui();

  bool running = true;
  while (running) {
    ImGuiIO& io = ImGui::GetIO();
    mousePressed[0] = mousePressed[1] = false;
    io.MouseWheel = 0;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    UpdateImGui();

    {
      static float f;
      ImGui::Text("Hello, world!");
      ImGui::SetWindowFontScale(2.0f);
    }

    // Rendering
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.8f, 0.6f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    SDL_GL_SwapWindow(window);
  }

  ImGui::Shutdown();
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
