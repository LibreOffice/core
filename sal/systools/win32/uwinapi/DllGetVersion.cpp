#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <malloc.h>

extern HMODULE      UWINAPI_BaseAddress;

// This function should be exported by every DLL that wants to provide it's version number.
// This code automaticly generates the information from the version resource

extern "C" HRESULT CALLBACK DllGetVersion( DLLVERSIONINFO *pdvi )
{
    TCHAR   szModulePath[MAX_PATH];
    BOOL    fSuccess = FALSE;

    if ( UWINAPI_BaseAddress && GetModuleFileName( UWINAPI_BaseAddress, szModulePath, MAX_PATH ) )
    {
        DWORD   dwHandle = 0;
        DWORD   dwSize = GetFileVersionInfoSize( szModulePath, &dwHandle );
        LPVOID  lpData = _alloca( dwSize );

        if ( GetFileVersionInfo( szModulePath, dwHandle, dwSize, lpData ) )
        {
            VS_FIXEDFILEINFO    *lpBuffer = NULL;
            UINT    uLen = 0;

            if ( VerQueryValue( lpData, TEXT("\\"), (LPVOID *)&lpBuffer, &uLen ) )
            {
                pdvi->dwMajorVersion = HIWORD( lpBuffer->dwFileVersionMS );
                pdvi->dwMinorVersion = LOWORD( lpBuffer->dwFileVersionMS );
                pdvi->dwBuildNumber = HIWORD( lpBuffer->dwFileVersionLS );
                pdvi->dwPlatformID = (lpBuffer->dwFileOS & VOS_NT) ? DLLVER_PLATFORM_NT : DLLVER_PLATFORM_WINDOWS;

                fSuccess = TRUE;
            }
        }
    }

    return fSuccess ? HRESULT_FROM_WIN32( GetLastError() ) : HRESULT_FROM_WIN32( NO_ERROR );
}

