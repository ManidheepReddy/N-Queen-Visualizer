// Interactive N‑Queens Visualizer using C++, OpenGL, GLFW, GLEW, and  ImGui 1.60
#include <iostream>
#include <vector>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "stb_image/stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"


void solveNQueens(int N, int row,
    std::vector<int>& cols,
    std::vector<std::vector<int>>& solutions)
{
    if (row == N) {
        solutions.push_back(cols);
        return;
    }
    for (int c = 0; c < N; c++) {
        bool ok = true;
        for (int r = 0; r < row; r++) {
            if (cols[r] == c
                || abs(r - row) == abs(cols[r] - c)) {
                ok = false;
                break;
            }
        }
        if (!ok) continue;
        cols[row] = c;
        solveNQueens(N, row + 1, cols, solutions);
    }
}


GLuint LoadTexture(const char* filepath) {
    int w, h, channels;
    unsigned char* data = stbi_load(filepath, &w, &h, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << "\n";
        return 0;
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D,
        0, GL_RGBA, w, h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    // linear filtering + clamp
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE);
    stbi_image_free(data);
    return tex;
}


int main() {

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window =
        glfwCreateWindow(800, 800,
            "N-Queens Visualizer",
            NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewInit();


    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();

    GLuint queenTex = LoadTexture("OpenGL\\res\\textures\\Flower3.png");
    if (!queenTex) {
        std::cerr << "Cannot continue without queen.png\n";
        return -1;
    }


    int N = 8;
    std::vector<std::vector<int>> solutions;
    std::vector<int> cols;
    float speed = 1.0f;        
    bool autoPlay = false;
    size_t current = 0;
    auto lastTime = std::chrono::high_resolution_clock::now();


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();

       
        ImGui::Begin("Controls");
        ImGui::SliderInt("Board size (N)", &N, 4, 12);
        if (ImGui::Button("Compute Solutions")) {
            solutions.clear();
            cols.assign(N, -1);
            solveNQueens(N, 0, cols, solutions);
            current = 0;
        }
        ImGui::Checkbox("Auto", &autoPlay);
        ImGui::SliderFloat("Speed (s)", &speed, 0.1f, 5.0f);
        ImGui::SameLine();
        if (ImGui::Button("Prev") && !solutions.empty()) {
            current = (current + solutions.size() - 1) % solutions.size();
        }
        ImGui::SameLine();
        if (ImGui::Button("Next") && !solutions.empty()) {
            current = (current + 1) % solutions.size();
        }
        if (!solutions.empty()) {
            ImGui::Text("Solution %zu / %zu", current + 1, solutions.size());
        }
        ImGui::End();

        if (autoPlay && !solutions.empty()) {
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - lastTime).count();
            if (dt >= speed) {
                current = (current + 1) % solutions.size();
                lastTime = now;
            }
        }


        ImGui::SetNextWindowPos({ 200, 0 });
        ImGui::SetNextWindowSize({ 600, 600 });
        ImGui::Begin("Board", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);

  
        float boardSize = ImGui::GetWindowSize().x;
        float cell = boardSize / N;

        auto draw = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetWindowPos();
        for (int i = 0; i <= N; i++) {
            float x = p.x + i * cell;
            float y = p.y + i * cell;
            draw->AddLine({ x, p.y }, { x, p.y + boardSize },
                IM_COL32(0, 0, 0, 255));
            draw->AddLine({ p.x, y }, { p.x + boardSize, y },
                IM_COL32(0, 0, 0, 255));
        }

     
        if (!solutions.empty()) {
            auto& sol = solutions[current];
            for (int r = 0; r < N; r++) {
                for (int c = 0; c < N; c++) {
                    ImVec2 cell_p0 = { p.x + c * cell, p.y + r * cell };
                    ImVec2 cell_p1 = { p.x + (c + 1) * cell,
                                       p.y + (r + 1) * cell };
                 
                    bool dark = (r + c) & 1;
                    draw->AddRectFilled(cell_p0, cell_p1,
                        dark ? IM_COL32(200, 200, 200, 255)
                        : IM_COL32(255, 255, 255, 255));
                    if (sol[r] == c) {
                       
                        ImGui::SetCursorScreenPos(cell_p0);
                        ImGui::Image(
                            (void*)(intptr_t)queenTex,
                            ImVec2(cell, cell));
                    }
                }
            }
        }

        ImGui::End(); 

     
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

  
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
