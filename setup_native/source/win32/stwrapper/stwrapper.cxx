/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#define WIN32_LEAN_AND_MEAN

#ifdef _MSC_VER
#pragma warning(disable:4668 4917) // disable warnings for system headers
#endif

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shlobj.h>
#include <tchar.h>

#include <stdio.h>
#include <sal/macros.h>

enum PathResult
{
    PATHRESULT_OK,
    PATHRESULT_API_NOT_SUPPORTED,
    PATHRESULT_EXE_NOT_FOUND
};

const int MAXCMDLINELEN = 32768;

static TCHAR       g_szSTInstallationPath[MAX_PATH] = TEXT("");
static TCHAR       g_szOperatingSystem[256]         = TEXT("");

static const TCHAR g_szSTExecutable[256]            = TEXT("stclient.exe");

//***************************************************************************

LONG RegReadValue( HKEY hBaseKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, LPVOID lpData, DWORD cbData )
{
    HKEY    hKey = NULL;
    LONG    lResult( 0 );

    lResult = RegOpenKeyEx( hBaseKey, lpSubKey, 0, KEY_QUERY_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        lResult = RegQueryValueEx( hKey, lpValueName, NULL, NULL, (LPBYTE)lpData, &cbData );
        RegCloseKey( hKey );
    }

    return lResult;
}

//***************************************************************************

static LPTSTR *GetCommandArgs( int *pArgc )
{
#ifdef UNICODE
    return CommandLineToArgvW( GetCommandLineW(), pArgc );
#else
    *pArgc = __argc;
    return __argv;
#endif
}

//***************************************************************************

static bool IsSupportedPlatform()
{
    OSVERSIONINFO aOsVersion;

    ZeroMemory( &aOsVersion, sizeof( OSVERSIONINFO ));
    aOsVersion.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

    // Try to determine OS version
    if ( GetVersionEx( &aOsVersion ))
    {
        switch ( aOsVersion.dwPlatformId )
        {
            case VER_PLATFORM_WIN32_NT:     // Windows NT based
                return true;

            case VER_PLATFORM_WIN32_WINDOWS: // Windows Me/98/95.
            case VER_PLATFORM_WIN32s:        // Win32s
                return false;

            default:
                return false;
        }
    }

    return false;
}

//***************************************************************************

static LPCTSTR GetOperatingSystemString()
{
    OSVERSIONINFO aOsVersion;

    ZeroMemory( &aOsVersion, sizeof( OSVERSIONINFO ));
    aOsVersion.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

    _tcscpy( g_szOperatingSystem, TEXT( "Microsoft Windows" ));

    // Try to determine OS version
    if ( GetVersionEx( &aOsVersion ))
    {
        switch ( aOsVersion.dwPlatformId )
        {
            // Test for the Windows NT product family.
            case VER_PLATFORM_WIN32_NT:
            {
                if ( aOsVersion.dwMajorVersion == 3 )
                {
                    _tcscat( g_szOperatingSystem, TEXT( " NT 3." ));
                    if ( aOsVersion.dwMinorVersion == 0 )
                        _tcscat( g_szOperatingSystem, TEXT( "0" ));
                    else if ( aOsVersion.dwMinorVersion == 5 )
                        _tcscat( g_szOperatingSystem, TEXT( "5" ));
                    else if ( aOsVersion.dwMinorVersion == 51 )
                        _tcscat( g_szOperatingSystem, TEXT( "51" ));
                }
                else if ( aOsVersion.dwMajorVersion == 4 )
                    _tcscat( g_szOperatingSystem, TEXT( " NT 4.0" ));
                else if ( aOsVersion.dwMajorVersion == 5 )
                {
                    if ( aOsVersion.dwMinorVersion == 0 )
                        _tcscat( g_szOperatingSystem, TEXT( " 2000" ));
                    else if ( aOsVersion.dwMinorVersion == 1 )
                        _tcscat( g_szOperatingSystem, TEXT( " XP" ));
                    else if ( aOsVersion.dwMinorVersion == 2 )
                        _tcscat( g_szOperatingSystem, TEXT( " Server 2003" ));
                }
                else if ( aOsVersion.dwMajorVersion == 6 )
                {
                    if ( aOsVersion.dwMinorVersion == 0 )
                        _tcscat( g_szOperatingSystem, " Vista" );
                }
            }
            break;

            // Test for the Windows Me/98/95.
            case VER_PLATFORM_WIN32_WINDOWS:
            {
                if ( aOsVersion.dwMinorVersion == 0 )
                  _tcscat( g_szOperatingSystem, TEXT( " 95" ));
                else if ( aOsVersion.dwMinorVersion == 10 )
                  _tcscat( g_szOperatingSystem, TEXT( " 98" ));
                else if ( aOsVersion.dwMinorVersion == 90 )
                  _tcscat( g_szOperatingSystem, TEXT( " Me" ));
            }
            break;
        }
    }

    return g_szOperatingSystem;
}

//***************************************************************************

static bool FileExists( LPCTSTR lpPathToFile )
{
    bool            bResult = false;
    HANDLE          hFind;
    WIN32_FIND_DATA FindFileData;

    hFind = FindFirstFile( lpPathToFile, &FindFileData );

    if ( hFind != INVALID_HANDLE_VALUE )
    {
        FindClose( hFind );
        bResult = true;
    }

    return bResult;
}

//***************************************************************************

static bool GetProgramFilesFolder( LPTSTR strPath )
{
    bool      bRet = false;
    HINSTANCE hLibrary;

    if (( hLibrary = LoadLibrary( "shell32.dll" )) != NULL )
    {
        BOOL (WINAPI *pSHGetSpecialFolderPathA)( HWND, LPSTR, int, BOOL );

        pSHGetSpecialFolderPathA = (BOOL (WINAPI *)(HWND, LPSTR, int, BOOL))GetProcAddress( hLibrary, "SHGetSpecialFolderPathA" );

        if ( pSHGetSpecialFolderPathA )
        {
            if ( pSHGetSpecialFolderPathA( NULL, strPath, CSIDL_PROGRAM_FILES, TRUE ))
                bRet = true;
        }
    }

    FreeLibrary( hLibrary );

    return ( bRet );
}

//***************************************************************************

static PathResult RetrieveExecutablePath( LPTSTR szExecutablePath )
{
    PathResult eRet = PATHRESULT_API_NOT_SUPPORTED;
    TCHAR szProgramFilesFolder[MAX_PATH];

    if ( GetProgramFilesFolder( szProgramFilesFolder ))
    {
        size_t nLen = _tcslen( szProgramFilesFolder );
        if ( nLen > 0 )
        {
            _tcscpy( szExecutablePath, szProgramFilesFolder );
            if ( szProgramFilesFolder[nLen-1] != '\\' )
                _tcscat( szExecutablePath, TEXT( "\\" ));
            _tcscat( szExecutablePath, TEXT( "Sun\\servicetag\\" ));
            _tcscat( szExecutablePath, g_szSTExecutable );
            eRet = FileExists( szExecutablePath ) ? PATHRESULT_OK : PATHRESULT_EXE_NOT_FOUND;
        }
    }

    return eRet;
}

//***************************************************************************

static void SafeCopy( LPTSTR lpTarget, LPCSTR lpSource, size_t nMaxLen )
{
    size_t nLen  = _tcslen( lpSource );
    size_t nCopy = ( nLen < size_t( nMaxLen-1 )) ? nLen : nMaxLen-1;
    _tcsncpy( lpTarget, lpSource, nMaxLen-1 );
    *(lpTarget+nCopy) = 0;
}

//***************************************************************************

int WINAPI _tWinMain( HINSTANCE /*hInstance*/, HINSTANCE, LPTSTR, int )
{
    const DWORD ERR_NO_RECORDS_FOUND = 225;
    const DWORD ERR_DUP_RECORD       = 226;

    DWORD dwExitCode = (DWORD)1;

    int     nArgs  = 0;
    LPTSTR* lpArgs = GetCommandArgs( &nArgs );

    if ( !IsSupportedPlatform() )
    {
        // Return 0 for a successful run on not supported platforms
        // We don't want that the Office tries to start us forever.
        return 0;
    }

    if ( nArgs >= 11 )
    {
        TCHAR szTargetURN[1024]         = {0};
        TCHAR szProductName[1024]       = {0};
        TCHAR szProductVersion[1024]    = {0};
        TCHAR szParentProductName[1024] = {0};
        TCHAR szProductSource[1024]     = {0};
        TCHAR szInstanceURN[1024]       = {0};

//      -i)  INSTANCE_URN="$2"; shift;;
//      -t)  TARGET_URN="$2"; shift;;
//      -p)  PRODUCT_NAME="$2"; shift;;
//      -e)  PRODUCT_VERSION="$2"; shift;;
//      -P)  PARENT_PRODUCT_NAME="$2"; shift;;
//      -S)  PRODUCT_SOURCE="$2"; shift;;
//      "usage: $0 [-i <instance urn>] -p <product name> -e <product version> -t <urn> -S <source> -P <parent product name>"

        int i = 1;
        while ( i < nArgs )
        {
            LPTSTR lpArg = lpArgs[i];
            if ( _tcslen( lpArg ) >= 2 )
            {
                if ( lpArg[0] == '-' )
                {
                    switch ( lpArg[1] )
                    {
                        case 'i':
                        {
                            if ( i < nArgs )
                                ++i;
                            SafeCopy( szInstanceURN, lpArgs[i], SAL_N_ELEMENTS( szInstanceURN ));
                            break;
                        }

                        case 't':
                        {
                            if ( i < nArgs )
                                ++i;
                            SafeCopy( szTargetURN, lpArgs[i], SAL_N_ELEMENTS( szTargetURN ));
                            break;
                        }
                        case 'p':
                        {
                            if ( i < nArgs )
                                ++i;
                            SafeCopy( szProductName, lpArgs[i], SAL_N_ELEMENTS( szProductName ));
                            break;
                        }
                        case 'e':
                        {
                            if ( i < nArgs )
                                ++i;
                            SafeCopy( szProductVersion, lpArgs[i], SAL_N_ELEMENTS( szProductVersion ));
                            break;
                        }
                        case 'P':
                        {
                            if ( i < nArgs )
                                ++i;
                            SafeCopy( szParentProductName, lpArgs[i], SAL_N_ELEMENTS( szParentProductName ));
                            break;
                        }
                        case 'S':
                        {
                            if ( i < nArgs )
                                ++i;
                            SafeCopy( szProductSource, lpArgs[i], SAL_N_ELEMENTS( szProductSource ));
                            break;
                        }

                        default:
                            break;
                    } // switch
                }
            }

            ++i;
        }

        if ( RetrieveExecutablePath( g_szSTInstallationPath ) == PATHRESULT_OK )
        {
            BOOL bSuccess = TRUE;
            BOOL bProcessStarted = FALSE;

            STARTUPINFO         aStartupInfo;
            PROCESS_INFORMATION aProcessInfo;
            LPTSTR              lpCommandLine = 0;

            ZeroMemory( &aStartupInfo, sizeof( aStartupInfo ));
            aStartupInfo.cb = sizeof( aStartupInfo );
            ZeroMemory( &aProcessInfo, sizeof( aProcessInfo ));

            if ( _tcslen( szInstanceURN ) == 0 )
            {
                // TEST=`${STCLIENT} -f -t ${TARGET_URN}`
                lpCommandLine = new TCHAR[MAXCMDLINELEN];

                _tcscpy( lpCommandLine, TEXT( "\"" ));
                _tcscat( lpCommandLine, g_szSTInstallationPath );
                _tcscat( lpCommandLine, TEXT( "\"" ));
                _tcscat( lpCommandLine, TEXT( " -f" ));
                _tcscat( lpCommandLine, TEXT( " -t "));
                _tcscat( lpCommandLine, TEXT( "\"" ));
                _tcscat( lpCommandLine, szTargetURN );
                _tcscat( lpCommandLine, TEXT( "\"" ));

                bSuccess = CreateProcess(
                                   NULL,
                                   lpCommandLine,
                                   NULL,
                                   NULL,
                                   TRUE,
                                   CREATE_NO_WINDOW,
                                   NULL,
                                   NULL,
                                   &aStartupInfo,
                                   &aProcessInfo );

                bProcessStarted = TRUE;

                // wait until process ends to receive exit code
                WaitForSingleObject( aProcessInfo.hProcess, INFINITE );

                delete []lpCommandLine;
            }

            if ( bSuccess )
            {
                DWORD dwSTClientExitCode( ERR_NO_RECORDS_FOUND );
                if ( bProcessStarted )
                {
                    GetExitCodeProcess( aProcessInfo.hProcess, &dwSTClientExitCode );
                    dwSTClientExitCode &= 0x000000ff;

                    CloseHandle( aProcessInfo.hProcess );
                    CloseHandle( aProcessInfo.hThread );
                }

                if ( dwSTClientExitCode == ERR_NO_RECORDS_FOUND )
                {
                    lpCommandLine = new TCHAR[MAXCMDLINELEN];

                    _tcscpy( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, g_szSTInstallationPath );
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, TEXT( " -a" ));
                    if ( _tcslen( szInstanceURN ) > 0 )
                    {
                        _tcscat( lpCommandLine, TEXT( " -i " ));
                        _tcscat( lpCommandLine, TEXT( "\"" ));
                        _tcscat( lpCommandLine, szInstanceURN );
                        _tcscat( lpCommandLine, TEXT( "\"" ));
                    }
                    _tcscat( lpCommandLine, TEXT( " -p " ));
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, szProductName );
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, TEXT( " -e " ));
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, szProductVersion );
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, TEXT( " -t " ));
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, szTargetURN );
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, TEXT( " -S " ));
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, szProductSource );
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, TEXT( " -P " ));
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, szParentProductName );
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, TEXT( " -m \"The Document Foundation\"" ));
                    _tcscat( lpCommandLine, TEXT( " -A " ));
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, GetOperatingSystemString() );
                    _tcscat( lpCommandLine, TEXT( "\"" ));
                    _tcscat( lpCommandLine, TEXT( " -z global" ));

                    ZeroMemory( &aStartupInfo, sizeof( aStartupInfo ));
                    aStartupInfo.cb = sizeof(aStartupInfo);
                    ZeroMemory( &aProcessInfo, sizeof( aProcessInfo ));

                    bSuccess = CreateProcess(
                                       NULL,
                                       lpCommandLine,
                                       NULL,
                                       NULL,
                                       TRUE,
                                       CREATE_NO_WINDOW,
                                       NULL,
                                       NULL,
                                       &aStartupInfo,
                                       &aProcessInfo );

                    delete []lpCommandLine;

                    // wait until process ends to receive exit code
                    WaitForSingleObject( aProcessInfo.hProcess, INFINITE );

                    dwSTClientExitCode = 0;
                    GetExitCodeProcess( aProcessInfo.hProcess, &dwSTClientExitCode );
                    dwSTClientExitCode &= 0x000000ff;

                    CloseHandle( aProcessInfo.hProcess );
                    CloseHandle( aProcessInfo.hThread );

                    if ( !bSuccess )
                        dwExitCode = 1; // couldn't start stclient process
                    else
                    {
                        if ( _tcslen( szInstanceURN ) > 0 )
                        {
                            // don't register again if we registered in a previous run
                            // or we called stclient successfully.
                            if (( dwSTClientExitCode == ERR_DUP_RECORD ) ||
                                ( dwSTClientExitCode == 0 ))
                                dwExitCode = 0;
                            else
                                dwExitCode = 1; // other errors
                        }
                        else
                            dwExitCode = ( dwSTClientExitCode == 0 ) ? 0 : 1;
                    }
                }
                else if ( dwSTClientExitCode == 0 )
                    dwExitCode = 0; // already registered
                else
                    dwExitCode = 1; // other errors
            }
            else
                dwExitCode = 1; // couldn't start stclient
        }
        else
            dwExitCode = 1; // no executable found
    }
    else
        dwExitCode = 0; // wrong number of arguments

    return dwExitCode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
