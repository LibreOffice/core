#include "macros.h"
#include <tlhelp32.h>


IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Module32First, (HANDLE hSnapshot, LPMODULEENTRY32 lpme ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Module32FirstW, (HANDLE hSnapshot, LPMODULEENTRY32W lpme ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Module32Next, (HANDLE hSnapshot, LPMODULEENTRY32 lpme ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Module32NextW, (HANDLE hSnapshot, LPMODULEENTRY32W lpme ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Process32First, (HANDLE hSnapshot, LPPROCESSENTRY32 lppe ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Process32FirstW, (HANDLE hSnapshot, LPPROCESSENTRY32W lppe ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Process32Next, (HANDLE hSnapshot, LPPROCESSENTRY32 lppe ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Process32NextW, (HANDLE hSnapshot, LPPROCESSENTRY32W lppe ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, HANDLE, WINAPI, CreateToolhelp32Snapshot, (DWORD dwFlags, DWORD th32ProcessID ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return NULL;
}
