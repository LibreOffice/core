#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>


HMODULE     UWINAPI_BaseAddress = NULL;
const CHAR  szUnicowsModuleName[] = "UNICOWS.DLL";

static HMODULE WINAPI _LoadUnicowsLibrary(VOID)
{
    CHAR        szModulePath[MAX_PATH];
    HMODULE     hModuleUnicows = NULL;

    // First search in the same directory as UWINAPI.DLL was loaded from. This is because
    // UWINAPI.DLL not always resides in the same directory as the actual application.

    if ( UWINAPI_BaseAddress && GetModuleFileNameA( UWINAPI_BaseAddress, szModulePath, MAX_PATH ) )
    {
        char    *lpLastBkSlash = strrchr( szModulePath, '\\' );

        if ( lpLastBkSlash )
        {
            size_t  nParentDirSize = lpLastBkSlash - szModulePath + 1;
            LPSTR   lpUnicowsModulePath = (LPTSTR)_alloca( nParentDirSize + sizeof(szUnicowsModuleName) );

            if ( lpUnicowsModulePath )
            {
                strncpy( lpUnicowsModulePath, szModulePath, nParentDirSize );
                strcat( lpUnicowsModulePath, szUnicowsModuleName );

                hModuleUnicows = LoadLibraryA( lpUnicowsModulePath );
            }
        }
    }

    // Search at the common places

    if ( !hModuleUnicows )
        hModuleUnicows = LoadLibraryA(szUnicowsModuleName);

    return hModuleUnicows;
}

static HMODULE WINAPI LoadUnicowsLibrary(VOID)
{
    HMODULE hModuleUnicows;
    int     idMsg = IDOK;

    do
    {
        hModuleUnicows = _LoadUnicowsLibrary();

        if ( !hModuleUnicows )
        {
            LPVOID lpMsgBuf;

            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                ERROR_DLL_NOT_FOUND /* GetLastError() */,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPSTR)&lpMsgBuf,
                0,
                NULL
            );
            // Process any inserts in lpMsgBuf.
            CHAR szModuleFileName[MAX_PATH];

            GetModuleFileNameA( NULL, szModuleFileName, sizeof(szModuleFileName) );
            LPSTR   lpMessage = (LPSTR)_alloca( strlen( (LPCSTR)lpMsgBuf ) + sizeof(szUnicowsModuleName) + 1 );
            strcpy( lpMessage, (LPCSTR)lpMsgBuf );
            strcat( lpMessage, "\n" );
            strcat( lpMessage, szUnicowsModuleName );
            // Free the buffer.
            LocalFree( lpMsgBuf );
            // Display the string.
            idMsg = MessageBoxA( NULL, lpMessage,
                szModuleFileName, MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_TASKMODAL );

            if ( IDABORT == idMsg )
                TerminateProcess( GetCurrentProcess(), 255 );
        }
    } while ( !hModuleUnicows && IDRETRY == idMsg );

    return hModuleUnicows;
}

extern "C" FARPROC _PfnLoadUnicows = (FARPROC)LoadUnicowsLibrary;

extern "C" BOOL WINAPI DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        UWINAPI_BaseAddress = hModule;
        return DisableThreadLibraryCalls( hModule );
    default:
        return TRUE;
    }

}






