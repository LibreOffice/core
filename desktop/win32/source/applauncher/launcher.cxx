#include "launcher.hxx"

#ifndef _WINDOWS_
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif


#include <stdlib.h>
#include <malloc.h>


extern "C" int APIENTRY _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )
{
    // Retreive startup info

    STARTUPINFO aStartupInfo;

    ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
    aStartupInfo.cb = sizeof( aStartupInfo );
    GetStartupInfo( &aStartupInfo );

    // Retrieve command line

    LPTSTR  lpCommandLine = GetCommandLine();

    LPTSTR  *ppArguments = NULL;
    int     nArguments = 0;

    ppArguments = GetArgv( &nArguments );

    if ( 1 == nArguments )
    {
        lpCommandLine = (LPTSTR)_alloca( sizeof(_TCHAR) * (_tcslen(lpCommandLine) + _tcslen(APPLICATION_SWITCH) + 2) );

        _tcscpy( lpCommandLine, GetCommandLine() );
        _tcscat( lpCommandLine, _T(" ") );
        _tcscat( lpCommandLine, APPLICATION_SWITCH );
    }


    // Calculate application name

    TCHAR   szApplicationName[MAX_PATH];
    TCHAR   szDrive[MAX_PATH];
    TCHAR   szDir[MAX_PATH];
    TCHAR   szFileName[MAX_PATH];
    TCHAR   szExt[MAX_PATH];

    GetModuleFileName( NULL, szApplicationName, MAX_PATH );
    _tsplitpath( szApplicationName, szDrive, szDir, szFileName, szExt );
    _tmakepath( szApplicationName, szDrive, szDir, OFFICE_IMAGE_NAME, _T(".exe") );

    PROCESS_INFORMATION aProcessInfo;

    BOOL    fSuccess = CreateProcess(
        szApplicationName,
        lpCommandLine,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &aStartupInfo,
        &aProcessInfo );

    if ( fSuccess )
    {
        CloseHandle( aProcessInfo.hProcess );
        CloseHandle( aProcessInfo.hThread );

        return 0;
    }

    DWORD   dwError = GetLastError();

    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );

    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, NULL, MB_OK | MB_ICONERROR );

    // Free the buffer.
    LocalFree( lpMsgBuf );

    return GetLastError();
}

