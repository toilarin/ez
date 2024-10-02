#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <thread>
#include <vector>
#include <cmath>
#include <TlHelp32.h>
#include <ctime>

// Các biến toàn cục cho Direct3D
LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev;
LPD3DXFONT d3dxFont;
HWND gameHWND;

// Cấu trúc đối thủ giả định
struct Enemy {
	float x, y;
	bool isVisible;
};

// Danh sách đối thủ giả định
std::vector<Enemy> enemies = {
{400, 300, true},
{600, 400, true},
{800, 500, true}
};

// Hàm khởi tạo Direct3D
void InitD3D(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);

	D3DXCreateFont(d3ddev, 25, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &d3dxFont);
}

// Hàm vẽ vòng tròn hitbox
void DrawCircle(float x, float y, float radius, D3DCOLOR color)
{
	const int NUMPOINTS = 30;
	D3DXVECTOR2 Line[NUMPOINTS + 1];
	for (int i = 0; i < NUMPOINTS; ++i)
	{
		float theta = (2 * D3DX_PI * i) / NUMPOINTS;
		Line[i] = D3DXVECTOR2(x + radius * cos(theta), y + radius * sin(theta));
	}
	Line[NUMPOINTS] = Line[0];
	d3ddev->DrawPrimitiveUP(D3DPT_LINESTRIP, NUMPOINTS, &Line[0], sizeof(D3DXVECTOR2));
}

// Hàm vẽ text
void DrawText(LPCSTR text, int x, int y, D3DCOLOR color)
{
	RECT rect;
	SetRect(&rect, x, y, x + 200, y + 16);
	d3dxFont->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, color);
}

// Hàm vẽ ESP
void RenderESP()
{
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	d3ddev->BeginScene();

	for (const auto& enemy : enemies)
	{
		if (enemy.isVisible)
		{
			DrawCircle(enemy.x, enemy.y, 50, D3DCOLOR_XRGB(255, 0, 0)); // Vẽ hitbox
			DrawText("Enemy", enemy.x, enemy.y - 60, D3DCOLOR_XRGB(0, 255, 0)); // Vẽ text ESP
		}
	}

	d3ddev->EndScene();
	d3ddev->Present(NULL, NULL, NULL, NULL);
}

// Hàm tự động nhấn phím
void PressKeyDown(WORD key)
{
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = key;
	input.ki.dwFlags = 0; // Key press
	SendInput(1, &input, sizeof(INPUT));
}

// Hàm tự động thả phím
void PressKeyUp(WORD key)
{
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = key;
	input.ki.dwFlags = KEYEVENTF_KEYUP; // Key release
	SendInput(1, &input, sizeof(INPUT));
}

// Trạng thái cho combo
enum ComboState {
	IDLE,
	ATTACK1,
	ATTACK2,
	FINISH
};

// Biến lưu trạng thái hiện tại của combo
ComboState currentComboState = IDLE;
DWORD lastComboTime = 0;

// Hàm tự động tạo combo thông minh
void AutoComboAI()
{
	srand((unsigned)time(0)); // Khởi tạo ngẫu nhiên

	while (true)
	{
		// Giả sử nhân vật ở tọa độ (400, 300) và khoảng cách ngắn nhất là 100
		float characterX = 400;
		float characterY = 300;
		float minDistance = 100.0f;

		// Tìm kiếm đối thủ ở gần nhất
		for (const auto& enemy : enemies)
		{
			float distance = sqrt(pow(enemy.x - characterX, 2) + pow(enemy.y - characterY, 2));
			if (enemy.isVisible && distance < minDistance)
			{
				// Thực hiện combo dựa trên trạng thái hiện tại
				switch (currentComboState)
				{
				case IDLE:
					if (GetTickCount() - lastComboTime > 300 && rand() % 2 == 0)
					{
						PressKeyDown(VK_UP); // Di chuyển lên
						PressKeyDown(0x4A); // Tấn công đầu tiên (phím J)
						Sleep(30);
						PressKeyUp(VK_UP); // Thả di chuyển lên
						PressKeyUp(0x4A); // Thả phím J
						currentComboState = ATTACK1;
						lastComboTime = GetTickCount();
					}
					break;

				case ATTACK1:
					if (GetTickCount() - lastComboTime > 300 && rand() % 2 == 0)
					{
						PressKeyDown(VK_DOWN); // Di chuyển xuống
							PressKeyDown(0x4B); // Tấn công thứ hai (phím K)
						Sleep(30);
						PressKeyUp(VK_DOWN); // Thả di chuyển xuống
						PressKeyUp(0x4B); // Thả phím K
						currentComboState = ATTACK2;
						lastComboTime = GetTickCount();
					}
					break;

				case ATTACK2:
					if (GetTickCount() - lastComboTime > 300 && rand() % 2 == 0)
					{
						PressKeyDown(VK_LEFT); // Di chuyển trái
						PressKeyDown(0x4C); // Tấn công hoàn tất (phím L)
						Sleep(30);
						PressKeyUp(VK_LEFT); // Thả di chuyển trái
						PressKeyUp(0x4C); // Thả phím L
						currentComboState = FINISH;
						lastComboTime = GetTickCount();
					}
					break;

				case FINISH:
					if (GetTickCount() - lastComboTime > 500)
					{
						currentComboState = IDLE; // Reset combo
					}
					break;
				}
			}
		}
		Sleep(10); // Nghỉ để giảm tải CPU
	}
}

// Tự động nhảy
void AutoJump() {
	while (true) {
		PressKeyDown(VK_SPACE); // Nhảy
		Sleep(30);
		PressKeyUp(VK_SPACE); // Thả phím nhảy
		Sleep(500); // Nghỉ giữa mỗi lần nhảy
	}
}

// Tự động né
void AutoDodge() {
	while (true) {
		PressKeyDown(VK_SHIFT); // Né (phím Shift)
		Sleep(30);
		PressKeyUp(VK_SHIFT); // Thả phím né
		Sleep(3000); // Né mỗi 3 giây
	}
}

// Tự động di chuyển
void AutoMove() {
	while (true) {
		PressKeyDown(0x41); // Di chuyển sang trái (phím A)
		Sleep(200);
		PressKeyUp(0x41); // Thả phím A
		PressKeyDown(0x44); // Di chuyển sang phải (phím D)
		Sleep(200);
		PressKeyUp(0x44); // Thả phím D
	}
}

// Thread chính
DWORD WINAPI MainThread(LPVOID param)
{
	gameHWND = FindWindowA(NULL, "Brawlhalla");
	InitD3D(gameHWND);

	std::thread espThread([]() {
		while (true)
		{
			RenderESP();
			Sleep(10); // Giảm tải CPU
		}
		});
	espThread.detach();

	std::thread jumpThread(AutoJump);
	std::thread comboThread(AutoComboAI);
	std::thread dodgeThread(AutoDodge);
	std::thread moveThread(AutoMove);

	jumpThread.detach();
	comboThread.detach();
	dodgeThread.detach();
	moveThread.detach();

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}