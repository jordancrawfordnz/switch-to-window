#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <winuser.h>

void SetForegroundWindowInternal(HWND hWnd)
{
	if (!::IsWindow(hWnd)) return;

	BYTE keyState[256] = { 0 };
	//to unlock SetForegroundWindow we need to imitate Alt pressing
	if (::GetKeyboardState((LPBYTE)&keyState))
	{
		if (!(keyState[VK_MENU] & 0x80))
		{
			::keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		}
	}

	::SetForegroundWindow(hWnd);

	if (::GetKeyboardState((LPBYTE)&keyState))
	{
		if (!(keyState[VK_MENU] & 0x80))
		{
			::keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		}
	}
}

// From: http://stackoverflow.com/questions/1888863/how-to-get-main-window-handle-from-process-id
struct handle_data {
	unsigned long process_id;
	HWND best_handle;
};

BOOL is_main_window(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle)) {
		return TRUE;
	}
	data.best_handle = handle;
	return FALSE;
}

HWND find_main_window(unsigned long process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.best_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.best_handle;
}

// Originally from: http://csharpapprentice.blogspot.co.nz/2013/03/ok-ok-its-c.html

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Invalid arguments provided.");
		return 1;
	}
	int processId = atoi(argv[1]);
	HWND processMainWindow = find_main_window(processId);
	if (processMainWindow == NULL)
	{
		printf("Process is not running.");
		return 2;
	}
	SetForegroundWindowInternal(processMainWindow);
	printf("Switched to process successfully.");
	return 0;
}
