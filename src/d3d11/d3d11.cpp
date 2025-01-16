/*
    CONTRIBUTORS:
        Sean Pesce

*/

#include "d3d11/main.h"
#include <string>

#include "d3d11/main.h"
#include <string>
#include <stdexcept>
#include <windows.h>
#include <sstream>

namespace d3d11 {

HMODULE chain = NULL;
FARPROC functions[func_count];

std::string GetLastErrorAsString()
{
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
    {
        return std::string(); // No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

BOOL IsWow64()
{
    typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = nullptr;
    BOOL bIsWow64 = FALSE;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

    if (fnIsWow64Process && !fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
    {
        throw std::runtime_error("Failed to determine if the process is running under WOW64: " + GetLastErrorAsString());
    }

    return bIsWow64;
}

void hook_exports()
{
    try
    {
        chain = LoadLibrary(IsWow64() ? "C:\\Windows\\SysWOW64\\d3d11.dll" : "C:\\Windows\\System32\\d3d11.dll");
        if (!chain)
        {
            throw std::runtime_error("Unable to locate original d3d11.dll (or compatible library to chain): " + GetLastErrorAsString());
        }

        int count = 0;
        for (int i = 0; i < d3d11::func_count; i++)
        {
            FARPROC func = GetProcAddress(chain, func_names[i]);
            if (func)
            {
                count++;
            }
            functions[i] = func;
        }

        HMODULE renderdoc = LoadLibrary("D:\\renderdoc.dll");
        if (!renderdoc)
        {
            throw std::runtime_error("D:\\renderdoc.dll NOT FOUND: " + GetLastErrorAsString());
        }
    }
    catch (const std::exception& e)
    {
        MessageBox(NULL, e.what(), "ERROR", MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
}

} // namespace d3d11
