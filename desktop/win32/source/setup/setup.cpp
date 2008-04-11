/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setup.cpp,v $
 * $Revision: 1.14 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#define WIN // scope W32 API

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <tchar.h>
#include <assert.h>
#include <shlwapi.h>
#include <new>
#include <time.h>
#include <mbctype.h>
#include <locale.h>

#include "strsafe.h"

#include "setup.hxx"

#include "resource.h"

//--------------------------------------------------------------------------

#define MAX_STR_LENGTH      1024
#define MAX_LANGUAGE_LEN      80
#define MAX_STR_CAPTION      256
#define VERSION_SIZE          80
#define SECTION_SETUP       TEXT( "Setup" )
#define SECTION_LANGUAGE    TEXT( "Languages" )
#define PRODUCT_NAME_VAR    TEXT( "%PRODUCTNAME" )

#define ERROR_SHOW_USAGE      -2

#define PARAM_SETUP_USED    TEXT( " SETUP_USED=1 " )
#define PARAM_PACKAGE       TEXT( "/I " )
#define PARAM_ADMIN         TEXT( "/A " )
#define PARAM_TRANSFORM     TEXT( " TRANSFORMS=" )
#define PARAM_REBOOT        TEXT( " REBOOT=Force" )

#define PARAM_RUNNING       TEXT( "ignore_running" )

#define MSI_DLL             TEXT( "msi.dll" )
#define ADVAPI32_DLL        TEXT( "advapi32.dll" )
#define PROFILE_NAME        TEXT( "setup.ini" )

#define MSIAPI_DllGetVersion     "DllGetVersion"
#define ADVAPI32API_CheckTokenMembership "CheckTokenMembership"

typedef HRESULT (CALLBACK* PFnDllGetVersion)( DLLVERSIONINFO *pdvi);
typedef BOOL (WINAPI* PFnCheckTokenMembership)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);

#ifdef DEBUG
inline void OutputDebugStringFormat( LPCTSTR pFormat, ... )
{
    TCHAR    buffer[1024];
    va_list  args;

    va_start( args, pFormat );
    StringCchVPrintf( buffer, sizeof(buffer), pFormat, args );
    OutputDebugString( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCTSTR, ... )
{
}
#endif

//--------------------------------------------------------------------------

const TCHAR sInstKey[]       = TEXT( "Software\\Microsoft\\Windows\\CurrentVersion\\Installer" );
const TCHAR sInstLocValue[]  = TEXT( "InstallerLocation" );
const TCHAR sMsiDll[]        = TEXT( "\\msi.dll" );
const TCHAR sMsiExe[]        = TEXT( "\\msiexec.exe" );
const TCHAR sDelayReboot[]   = TEXT( " /c:\"msiinst /delayreboot\"" );
const TCHAR sMsiQuiet[]      = TEXT( " /q" );
const TCHAR sMemMapName[]    = TEXT( "Global\\MsiErrorObject" );

//--------------------------------------------------------------------------
SetupAppX::SetupAppX()
{
    m_hInst     = NULL;
    m_hMapFile  = NULL;
    m_pAppTitle = NULL;
    m_pCmdLine  = NULL;

    m_pDatabase = NULL;
    m_pInstMsiW = NULL;
    m_pInstMsiA = NULL;
    m_pReqVersion   = NULL;
    m_pProductName  = NULL;
    m_pAdvertise    = NULL;
    m_pTmpName      = NULL;
    m_pLogFile      = NULL;
    m_pModuleFile   = NULL;
    m_pMSIErrorCode = NULL;

    m_pErrorText    = new TCHAR[ MAX_STR_LENGTH ];
    m_pErrorText[0] = '\0';

    m_nLanguageID     = 0;
    m_nLanguageCount  = 0;
    m_ppLanguageList  = NULL;

    m_bQuiet          = false;
    m_bAdministrative = false;
    m_bIgnoreAlreadyRunning = false;
}

//--------------------------------------------------------------------------
SetupAppX::~SetupAppX()
{
    if ( m_ppLanguageList )
    {
        for ( int i = 0; i < m_nLanguageCount; i++ )
            if ( m_ppLanguageList[i] )
                delete m_ppLanguageList[ i ];
        delete [] m_ppLanguageList;
    }

    time_t aTime;
    time( &aTime );
    tm *pTime = localtime( &aTime );   // Convert time to struct tm form

    Log( TEXT( "End: %s\n\r\n\r\n" ), _tasctime( pTime ) );

    if ( m_pLogFile ) fclose( m_pLogFile );

    if ( m_pTmpName )
    {
        _tremove( m_pTmpName );
        free( m_pTmpName );
    }

    if ( m_pMSIErrorCode ) UnmapViewOfFile( m_pMSIErrorCode );
    if ( m_hMapFile ) CloseHandle( m_hMapFile );

    if ( m_pAppTitle ) delete [] m_pAppTitle;
    if ( m_pDatabase ) delete [] m_pDatabase;
    if ( m_pInstMsiW ) delete [] m_pInstMsiW;
    if ( m_pInstMsiA ) delete [] m_pInstMsiA;
    if ( m_pReqVersion ) delete [] m_pReqVersion;
    if ( m_pProductName ) delete [] m_pProductName;
    if ( m_pAdvertise )   delete [] m_pAdvertise;
    if ( m_pLogFile )     delete [] m_pLogFile;
    if ( m_pErrorText )   delete [] m_pErrorText;
    if ( m_pModuleFile )  delete [] m_pModuleFile;
}

//--------------------------------------------------------------------------
boolean SetupAppX::Initialize( HINSTANCE hInst )
{
    m_pCmdLine  = WIN::GetCommandLine();
    m_hInst     = hInst;

    // Load our AppTitle (caption)
    m_pAppTitle     = new TCHAR[ MAX_STR_CAPTION ];
    m_pAppTitle[0]  = '\0';
    WIN::LoadString( hInst, IDS_APP_TITLE, m_pAppTitle, MAX_STR_CAPTION );

    // Obtain path we are running from
    m_pModuleFile       = new TCHAR[ MAX_PATH ];
    m_pModuleFile[ 0 ]  = '\0';

    if ( 0 == WIN::GetModuleFileName( hInst, m_pModuleFile, MAX_PATH ) )
    {
        SetError( WIN::GetLastError() );
        return false;
    }

    if ( ! GetCmdLineParameters( &m_pCmdLine ) )
        return false;

    m_hMapFile = CreateFileMapping(
                 INVALID_HANDLE_VALUE,      // use paging file
                 NULL,                      // default security
                 PAGE_READWRITE,            // read/write access
                 0,                         // max. object size
                 sizeof( int ),             // buffer size
                 sMemMapName );
    if ( m_hMapFile )
    {
        m_pMSIErrorCode = (int*) MapViewOfFile( m_hMapFile,  // handle to map object
                        FILE_MAP_ALL_ACCESS,   // read/write permission
                        0,
                        0,
                        sizeof( int ) );
        if ( m_pMSIErrorCode )
            *m_pMSIErrorCode = 0;
        else
            OutputDebugStringFormat( TEXT("Could not map view of file (%d).\n"), GetLastError() );
    }
    else
        OutputDebugStringFormat( TEXT("Could not create file mapping object (%d).\n"), GetLastError() );

    Log( TEXT("Starting: %s\r\n"), m_pModuleFile );
    Log( TEXT(" CommandLine=<%s>\r\n"), m_pCmdLine );

    if ( m_bQuiet )
        Log( TEXT(" Using quiet install mode\r\n") );

    time_t aTime;
    time( &aTime );
    tm* pTime = localtime( &aTime );
    Log( TEXT(" Begin: %s\n"), _tasctime( pTime ) );

    return true;
}

//--------------------------------------------------------------------------
boolean SetupAppX::GetProfileSection( LPCTSTR pFileName, LPCTSTR pSection,
                                      DWORD& rSize, LPTSTR *pRetBuf )
{
    if ( !rSize || !*pRetBuf )
    {
        rSize = 512;
        *pRetBuf = new TCHAR[ rSize ];
    }

    DWORD nRet = GetPrivateProfileSection( pSection, *pRetBuf, rSize, pFileName );

    if ( nRet && ( nRet + 2 > rSize ) ) // buffer was too small, retry with bigger one
    {
        if ( nRet < 32767 - 2 )
        {
            delete [] (*pRetBuf);
            rSize = nRet + 2;
            *pRetBuf = new TCHAR[ rSize ];

            nRet = GetPrivateProfileSection( pSection, *pRetBuf, rSize, pFileName );
        }
    }

    if ( !nRet )
    {
        SetError( WIN::GetLastError() );

        TCHAR sBuf[80];
        StringCchPrintf( sBuf, 80, TEXT("ERROR: GetPrivateProfileSection(): GetLastError returned %u\r\n"), GetError() );
        Log( sBuf );
        return false;
    }
    else if ( nRet + 2 > rSize )
    {
        SetError( ERROR_OUTOFMEMORY );
        Log( TEXT( "ERROR: GetPrivateProfileSection() out of memory\r\n" ) );
        return false;
    }

    Log( TEXT( " GetProfileSection read %s\r\n" ), pSection );

    return true;
}

//--------------------------------------------------------------------------
boolean SetupAppX::ReadProfile()
{
    boolean bRet = false;
    TCHAR *sProfilePath = 0;

    if ( GetPathToFile( PROFILE_NAME, &sProfilePath ) )
    {
        DWORD nSize = 0;
        LPTSTR pRetBuf = NULL;

        Log( TEXT( " Open ini file: <%s>\r\n" ), sProfilePath );

        bRet = GetProfileSection( sProfilePath, SECTION_SETUP, nSize, &pRetBuf );

        if ( !bRet )
        {
            LPTSTR pTmpFile = CopyIniFile( sProfilePath );
            delete [] sProfilePath;
            sProfilePath = pTmpFile;

            if ( sProfilePath )
            {
                SetError( ERROR_SUCCESS );

                Log( TEXT( " Could not open inifile, copied ini file to: <%s>\r\n" ), sProfilePath );
                bRet = GetProfileSection( sProfilePath, SECTION_SETUP, nSize, &pRetBuf );
            }
        }

        if ( bRet )
        {
            LPTSTR pCurLine = pRetBuf;
            while ( *pCurLine )
            {
                LPTSTR pName = 0;
                LPTSTR pValue = 0;

                pCurLine += GetNameValue( pCurLine, &pName, &pValue );

                if ( lstrcmpi( TEXT( "database" ), pName ) == 0 )
                {
                    m_pDatabase = pValue;
                    Log( TEXT( "    Database = %s\r\n" ), pValue );
                }
                else if ( lstrcmpi( TEXT( "instmsiw" ), pName ) == 0 )
                {
                    m_pInstMsiW = pValue;
                    Log( TEXT( "    instmsiw = %s\r\n" ), pValue );
                }
                else if ( lstrcmpi( TEXT( "instmsia" ), pName ) == 0 )
                {
                    m_pInstMsiA = pValue;
                    Log( TEXT( "    instmsia = %s\r\n" ), pValue );
                }
                else if ( lstrcmpi( TEXT( "msiversion" ), pName ) == 0 )
                {
                    m_pReqVersion = pValue;
                    Log( TEXT( "    msiversion = %s\r\n" ), pValue );
                }
                else if ( lstrcmpi( TEXT( "productname" ), pName ) == 0 )
                {
                    m_pProductName = pValue;
                    Log( TEXT( "    productname = %s\r\n" ), pValue );
                    m_pAppTitle = SetProdToAppTitle( m_pProductName );
                }
                else
                {
                    Log( TEXT( "Warning: unknown entry in profile <%s>\r\n" ), pName );
                    delete [] pValue;
                }
            }
        }

        if ( bRet && ( !m_pDatabase || !m_pInstMsiW || !m_pInstMsiA || !m_pReqVersion || !m_pProductName ) )
        {
            Log( TEXT( "ERROR: incomplete 'Setup' section in profile\r\n" ) );
            SetError( ERROR_INVALID_DATA );
            bRet = false;
        }

        if ( bRet )
            bRet = GetProfileSection( sProfilePath, SECTION_LANGUAGE, nSize, &pRetBuf );

        if ( bRet )
        {
            LPTSTR pName = 0;
            LPTSTR pValue = 0;
            LPTSTR pCurLine = pRetBuf;
            LPTSTR pLastChar;
            int  nNext = 0;

            // first line in this section should be the language count
            nNext = GetNameValue( pCurLine, &pName, &pValue );
            if ( lstrcmpi( TEXT( "count" ), pName ) == 0 )
            {
                Log( TEXT( "    Languages = %s\r\n" ), pValue );
                m_nLanguageCount = _tcstol( pValue, &pLastChar, 10 );
                pCurLine += nNext;
                delete [] pValue;
            }

            m_ppLanguageList = new LanguageDataX*[ m_nLanguageCount ];

            for ( int i=0; i < m_nLanguageCount; i++ )
            {
                if ( !*pCurLine )
                {
                    m_nLanguageCount = i;
                    break;
                }

                pCurLine += GetNameValue( pCurLine, &pName, &pValue );
                m_ppLanguageList[ i ] = new LanguageDataX( pValue );
                Log( TEXT( "    Language = %s\r\n" ), pValue );

                if ( m_ppLanguageList[ i ]->m_pTransform )
                    Log( TEXT( "      Transform = %s\r\n" ), m_ppLanguageList[ i ]->m_pTransform );

                delete [] pValue;
            }
        }

        if ( pRetBuf )
            delete [] pRetBuf;
    }

    if ( sProfilePath && ! m_pTmpName )
        delete [] sProfilePath;

    return bRet;
}

//--------------------------------------------------------------------------
boolean SetupAppX::GetPathToFile( TCHAR* pFileName, TCHAR** pPath )
{
    // generate the path to the file =  szModuleFile + FileName
    //   note: FileName is a relative path

    boolean bRet = true;

    int nTempPath = lstrlen( m_pModuleFile ) + lstrlen( pFileName ) + 2; // 1 for null terminator, 1 for back slash
    TCHAR* pTempPath = new TCHAR[ nTempPath ];

    // find 'setup.exe' in the path so we can remove it
    TCHAR *pFilePart = 0;
    if ( 0 == GetFullPathName( m_pModuleFile, nTempPath, pTempPath, &pFilePart ) )
    {
        SetError( WIN::GetLastError() );
        bRet = false;
    }
    else
    {
        if ( pFilePart )
            *pFilePart = '\0';

        StringCchCat( pTempPath, nTempPath, pFileName );

        int nPath = 2 * nTempPath;
        *pPath = new TCHAR[ nPath ];

        // normalize the path
        int nReturn = GetFullPathName( pTempPath, nPath, *pPath, &pFilePart );

        if ( nReturn > nPath )
        {
            // try again, with larger buffer
            delete [] (*pPath);
            nPath = nReturn;
            *pPath = new TCHAR[ nPath ];

            nReturn = GetFullPathName( pTempPath, nPath, *pPath, &pFilePart );
        }

        if ( 0 == nReturn )
        {
            // error -- invalid path
            SetError( WIN::GetLastError() );
            bRet = false;
        }
    }

    if ( bRet ) // check for the file's existence
    {
        DWORD dwFileAttrib = GetFileAttributes( *pPath );

        if (0xFFFFFFFF == dwFileAttrib)
        {
            StringCchCopy( m_pErrorText, MAX_STR_LENGTH, pFileName );
            SetError( ERROR_FILE_NOT_FOUND );
            bRet = false;
        }
    }

    delete [] pTempPath;
    return bRet;
}

//--------------------------------------------------------------------------
int SetupAppX::GetNameValue( TCHAR* pLine, TCHAR** pName, TCHAR** pValue )
{
    int nRet = lstrlen( pLine ) + 1;
    *pValue = 0;

    if ( nRet == 1 )
        return nRet;

    LPTSTR  pChar = pLine;
    LPTSTR  pLast = NULL;

    // Skip leading spaces.
    while (' ' == *pChar || '\t' == *pChar)
        pChar = CharNext( pChar );

    *pName = pChar;

    // look for the end of the name
    while( *pChar && (' ' != *pChar) &&
           ( '\t' != *pChar ) && ( '=' != *pChar ) )
        pChar = CharNext( pChar );

    if ( ! *pChar )
        return nRet;

    pLast = pChar;
    pChar = CharNext( pChar );
    *pLast = '\0';

    // look for the start of the value
    while( ( ' ' == *pChar ) || ( '\t' == *pChar ) ||
           ( '=' == *pChar ) )
        pChar = CharNext( pChar );

    int nValueLen = lstrlen( pChar ) + 1;
    *pValue = new TCHAR[ nValueLen ];

    if ( *pValue )
        StringCchCopy( *pValue, nValueLen, pChar );

    return nRet;
}

//--------------------------------------------------------------------------
boolean SetupAppX::ChooseLanguage( long& rLanguage )
{
    rLanguage = 0;

    if ( m_bQuiet )
        return true;

    // When there are none or only one language, there is nothing
    // to do here
    if ( m_nLanguageCount > 1 )
    {
        TCHAR *sString = new TCHAR[ MAX_LANGUAGE_LEN ];

        LANGID nUserDefLang = GetUserDefaultLangID();
        LANGID nSysDefLang = GetSystemDefaultLangID();

        int nUserPrimary = PRIMARYLANGID( nUserDefLang );
        int nSysPrimary = PRIMARYLANGID( nSysDefLang );

        long nUserIndex = -1;
        long nUserPrimIndex = -1;
        long nSystemIndex = -1;
        long nSystemPrimIndex = -1;
        long nParamIndex = -1;

        for ( long i=0; i<GetLanguageCount(); i++ )
        {
            long nLanguage = GetLanguageID( i );
            int nPrimary = PRIMARYLANGID( nLanguage );
            GetLanguageName( nLanguage, sString );
            Log( TEXT( "    Info: found Language: %s\r\n" ), sString );

            if ( nLanguage == nUserDefLang )
                nUserIndex = i;
            if ( nPrimary == nUserPrimary )
                nUserPrimIndex = i;
            if ( nLanguage == nSysDefLang )
                nSystemIndex = i;
            if ( nPrimary == nSysPrimary )
                nSystemPrimIndex = i;
            if ( m_nLanguageID && ( nLanguage == m_nLanguageID ) )
                nParamIndex = i;
        }

        if ( m_nLanguageID && ( nParamIndex == -1 ) )
        {
            Log( TEXT( "Warning: Language chosen with parameter -lang not found.\r\n" ) );
        }

        if ( nParamIndex != -1 )
        {
            Log( TEXT( "Info: Found language chosen with parameter -lang.\r\n" ) );
            rLanguage = GetLanguageID( nParamIndex );
        }
        else if ( nUserIndex != -1 )
        {
            Log( TEXT( "Info: Found user default language.\r\n" ) );
            rLanguage = GetLanguageID( nUserIndex );
        }
        else if ( nUserPrimIndex != -1 )
        {
            Log( TEXT( "Info: Found user default primary language.\r\n" ) );
            rLanguage = GetLanguageID( nUserPrimIndex );
        }
        else if ( nSystemIndex != -1 )
        {
            Log( TEXT( "Info: Found system default language.\r\n" ) );
            rLanguage = GetLanguageID( nSystemIndex );
        }
        else if ( nSystemPrimIndex != -1 )
        {
            Log( TEXT( "Info: Found system default primary language.\r\n" ) );
            rLanguage = GetLanguageID( nSystemPrimIndex );
        }
        else
        {
            Log( TEXT( "Info: Use default language from ini file.\r\n" ) );
            rLanguage = GetLanguageID( 0 );
        }
        delete [] sString;
    }

    return true;
}

//--------------------------------------------------------------------------
HMODULE SetupAppX::LoadMsiLibrary()
{
    HMODULE hMsi = NULL;
    HKEY    hInstKey = NULL;

    // find registered location of Msi.dll
    if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, sInstKey, 0, KEY_READ, &hInstKey ) )
    {
        long    nRet = ERROR_SUCCESS;
        TCHAR  *sMsiFolder = new TCHAR[ MAX_PATH + 1 ];
        DWORD   dwMsiFolderSize = MAX_PATH + 1;
        DWORD   dwType = 0;

        if ( ERROR_MORE_DATA == ( nRet = RegQueryValueEx( hInstKey, sInstLocValue, NULL,
                                                          &dwType, (BYTE*)sMsiFolder, &dwMsiFolderSize ) ) )
        {
            // try again with larger buffer
            delete [] sMsiFolder;
            sMsiFolder = new TCHAR[ dwMsiFolderSize ];

            nRet = RegQueryValueEx( hInstKey, sInstLocValue, NULL, &dwType,
                                    (BYTE*)sMsiFolder, &dwMsiFolderSize );
        }

        if ( ERROR_SUCCESS == nRet && dwType == REG_SZ && dwMsiFolderSize > 0 )
        {
            // load Msi.dll from registered location
            int nLength = lstrlen( sMsiDll ) + dwMsiFolderSize + 1; // use StringCchLength ?
            TCHAR *pMsiLocation = new TCHAR[ nLength ];

            if ( SUCCEEDED( StringCchCopy( pMsiLocation, nLength, sMsiFolder ) ) &&
                 SUCCEEDED( StringCchCat( pMsiLocation, nLength, sMsiDll ) ) )
            {
                hMsi = LoadLibrary( pMsiLocation );
            }
        }
    }

    if ( !hMsi ) // use the default location
    {
        hMsi = LoadLibrary( sMsiDll );
    }

    return hMsi;
}

//--------------------------------------------------------------------------
LPCTSTR SetupAppX::GetPathToMSI()
{
    LPTSTR  sMsiPath = NULL;
    HKEY    hInstKey = NULL;
    TCHAR  *sMsiFolder = new TCHAR[ MAX_PATH + 1 ];
    DWORD   nMsiFolderSize = MAX_PATH + 1;

    sMsiFolder[0] = '\0';

    // find registered location of Msi.dll
    if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, sInstKey, 0, KEY_READ, &hInstKey ) )
    {
        LONG    nRet = ERROR_SUCCESS;
        DWORD   dwType = 0;

        if ( ERROR_MORE_DATA == ( nRet = RegQueryValueEx( hInstKey, sInstLocValue, NULL,
                                                          &dwType, (BYTE*)sMsiFolder, &nMsiFolderSize ) ) )
        {
            // try again with larger buffer
            delete [] sMsiFolder;
            sMsiFolder = new TCHAR[ nMsiFolderSize ];

            nRet = RegQueryValueEx( hInstKey, sInstLocValue, NULL, &dwType,
                                    (BYTE*)sMsiFolder, &nMsiFolderSize );
        }

        if ( ERROR_SUCCESS != nRet || dwType != REG_SZ || nMsiFolderSize == 0 )
            sMsiFolder[0] = '\0';
    }

    if ( sMsiFolder[0] == '\0' ) // use the default location
    {
        Log( TEXT( "  Could not find path to msiexec.exe in registry" ) );

        DWORD nRet = WIN::GetSystemDirectory( sMsiFolder, nMsiFolderSize );
        if ( nRet > nMsiFolderSize )
        {
            delete [] sMsiFolder;
            sMsiFolder = new TCHAR[ nRet ];
            nMsiFolderSize = nRet;

            nRet = WIN::GetSystemDirectory( sMsiFolder, nMsiFolderSize );
        }
        if ( 0 == nRet )
        {
            sMsiFolder[0] = '\0';
            SetError( WIN::GetLastError() );
        }
        nMsiFolderSize = nRet;
    }

    if ( sMsiFolder[0] != '\0' )
    {
        int nLength = lstrlen( sMsiExe ) + lstrlen( sMsiFolder ) + 1;
        sMsiPath = new TCHAR[ nLength ];

        if ( FAILED( StringCchCopy( sMsiPath, nLength, sMsiFolder ) ) ||
             FAILED( StringCchCat( sMsiPath, nLength, sMsiExe ) ) )
        {
            delete [] sMsiPath;
            sMsiPath = NULL;
        }
    }

    if ( ! sMsiPath )
        Log( TEXT( "ERROR: Can't build path to msiexec.exe!" ) );

    return sMsiPath;
}

//--------------------------------------------------------------------------
boolean SetupAppX::LaunchInstaller( LPCTSTR pParam )
{
    LPCTSTR sMsiPath = GetPathToMSI();

    if ( !sMsiPath )
    {
        Log( TEXT( "ERROR: msiexec not found!" ) );
        SetError( ERROR_FILE_NOT_FOUND );
        return false;
    }

    STARTUPINFO         aSUI;
    PROCESS_INFORMATION aPI;

    Log( TEXT( " Will install using <%s>\r\n" ), sMsiPath );
    Log( TEXT( "   Prameters are: %s\r\n" ), pParam );

    ZeroMemory( (void*)&aPI, sizeof( PROCESS_INFORMATION ) );
    ZeroMemory( (void*)&aSUI, sizeof( STARTUPINFO ) );

    aSUI.cb          = sizeof(STARTUPINFO);
    aSUI.dwFlags     = STARTF_USESHOWWINDOW;
    aSUI.wShowWindow = SW_SHOW;

    DWORD nCmdLineLength = lstrlen( sMsiPath ) + lstrlen( pParam ) + 2;
    TCHAR *sCmdLine = new TCHAR[ nCmdLineLength ];

    if ( FAILED( StringCchCopy( sCmdLine, nCmdLineLength, sMsiPath ) ) ||
         FAILED( StringCchCat(  sCmdLine, nCmdLineLength, TEXT( " " ) ) ) ||
         FAILED( StringCchCat(  sCmdLine, nCmdLineLength, pParam ) ) )
    {
        delete [] sCmdLine;
        SetError( ERROR_INSTALL_FAILURE );
        return false;
    }

    if ( !WIN::CreateProcess( NULL, sCmdLine, NULL, NULL, FALSE,
                              CREATE_DEFAULT_ERROR_MODE, NULL, NULL,
                              &aSUI, &aPI ) )
    {
        Log( TEXT( "ERROR: Could not create process %s.\r\n" ), sCmdLine );
        SetError( WIN::GetLastError() );
        delete [] sCmdLine;
        return false;
    }

    DWORD nResult = WaitForProcess( aPI.hProcess );
    bool bRet = true;

    if( ERROR_SUCCESS != nResult )
    {
        Log( TEXT( "ERROR: While waiting for %s.\r\n" ), sCmdLine );
        SetError( nResult );
        bRet = false;
    }
    else
    {
        GetExitCodeProcess( aPI.hProcess, &nResult );
        SetError( nResult );

        if ( nResult != ERROR_SUCCESS )
        {
            TCHAR sBuf[80];
            StringCchPrintf( sBuf, 80, TEXT("Warning: msiexec returned %u.\r\n"), nResult );
            Log( sBuf );
        }
        else
            Log( TEXT( " Installation completed successfully.\r\n" ) );
    }

    CloseHandle( aPI.hProcess );

    delete [] sCmdLine;

    return bRet;
}

//--------------------------------------------------------------------------
boolean SetupAppX::Install( long nLanguage )
{
    LPTSTR pTransform = NULL;

    if ( nLanguage ) // look for transformation
    {
        for ( int i = 0; i < m_nLanguageCount; i++ )
        {
            if ( m_ppLanguageList[i]->m_nLanguageID == nLanguage )
            {
                if ( m_ppLanguageList[i]->m_pTransform )
                {
                    if ( !GetPathToFile( m_ppLanguageList[i]->m_pTransform,
                                        &pTransform ) )
                    {
                        Log( TEXT( "ERROR: Could not find transform <%s\r\n" ), m_ppLanguageList[i]->m_pTransform );
                        return false;
                    }
                }
                break;
            }
        }
    }

    TCHAR *pDataBasePath = NULL;

    if ( ! GetPathToFile( m_pDatabase, &pDataBasePath ) )
    {
        Log( TEXT( "ERROR: Could not find database <%s\r\n" ), m_pDatabase );
        SetError( ERROR_INSTALL_SOURCE_ABSENT );
        return false;
    }

    // we will always use the parameter setup used
    int nParLen = lstrlen( PARAM_SETUP_USED );

    if ( m_pAdvertise )
        nParLen += lstrlen( m_pAdvertise ) + 1;     // one for the space
    else
        nParLen += lstrlen( PARAM_PACKAGE );

    nParLen += lstrlen( pDataBasePath ) + 3;        // two quotes, one null

    if ( NeedReboot() )
        nParLen += lstrlen( PARAM_REBOOT );

    if ( pTransform )
    {
        nParLen += lstrlen( PARAM_TRANSFORM );
        nParLen += lstrlen( pTransform ) + 2;       // two quotes
    }

    if ( m_pCmdLine )
        nParLen += lstrlen( m_pCmdLine ) + 1;       // one for the space;

    TCHAR *pParams = new TCHAR[ nParLen ];

    StringCchCopy( pParams, nParLen, PARAM_SETUP_USED );

    if ( m_pAdvertise )
        StringCchCat( pParams, nParLen, m_pAdvertise );
    else if ( m_bAdministrative )
        StringCchCat( pParams, nParLen, PARAM_ADMIN );
    else
        StringCchCat( pParams, nParLen, PARAM_PACKAGE );

    StringCchCat( pParams, nParLen, TEXT( "\"" ) );
    StringCchCat( pParams, nParLen, pDataBasePath );
    StringCchCat( pParams, nParLen, TEXT( "\"" ) );

    if ( NeedReboot() )
        StringCchCat( pParams, nParLen, PARAM_REBOOT );

    if ( pTransform )
    {
        StringCchCat( pParams, nParLen, PARAM_TRANSFORM );
        StringCchCat( pParams, nParLen, TEXT( "\"" ) );
        StringCchCat( pParams, nParLen, pTransform );
        StringCchCat( pParams, nParLen, TEXT( "\"" ) );
    }

    if ( m_pCmdLine )
    {
        StringCchCat( pParams, nParLen, TEXT( " " ) );
        StringCchCat( pParams, nParLen, m_pCmdLine );
    }

    return LaunchInstaller( pParams );
}

//--------------------------------------------------------------------------
UINT SetupAppX::GetError() const
{
    UINT nErr = 0;

    if ( m_pMSIErrorCode )
        nErr = (UINT) *m_pMSIErrorCode;

    if ( nErr == 0 )
        nErr = m_uiRet;

    if ( nErr != 0 )
        OutputDebugStringFormat( TEXT("Setup will return error (%d).\n"), nErr );
    return nErr;
}

//--------------------------------------------------------------------------
void SetupAppX::DisplayError( UINT nErr ) const
{
    TCHAR sError[ MAX_STR_LENGTH ] = {0};
    TCHAR sTmp[ MAX_STR_LENGTH ] = {0};

    UINT  nMsgType = MB_OK | MB_ICONERROR;

    switch ( nErr )
    {
        case ERROR_SUCCESS:     break;  // 0

        case ERROR_FILE_NOT_FOUND:  // 2
                                WIN::LoadString( m_hInst, IDS_FILE_NOT_FOUND, sTmp, MAX_STR_LENGTH );
                                StringCchPrintf( sError, MAX_STR_LENGTH, sTmp, m_pErrorText );
                                break;
        case ERROR_INVALID_DATA:    // 13
                                WIN::LoadString( m_hInst, IDS_INVALID_PROFILE, sError, MAX_STR_LENGTH );
                                break;
        case ERROR_OUTOFMEMORY: WIN::LoadString( m_hInst, IDS_OUTOFMEM, sError, MAX_STR_LENGTH );
                                break;
        case ERROR_INSTALL_USEREXIT:
                                WIN::LoadString( m_hInst, IDS_USER_CANCELLED, sError, MAX_STR_LENGTH );
                                break;
        case ERROR_INSTALL_ALREADY_RUNNING: // 1618
                                WIN::LoadString( m_hInst, IDS_ALREADY_RUNNING, sError, MAX_STR_LENGTH );
                                break;
        case ERROR_INSTALL_SOURCE_ABSENT:
                                WIN::LoadString( m_hInst, IDS_NOMSI, sError, MAX_STR_LENGTH );
                                break;
        case ERROR_DS_INSUFF_ACCESS_RIGHTS: // 8344
                                WIN::LoadString( m_hInst, IDS_REQUIRES_ADMIN_PRIV, sError, MAX_STR_LENGTH );
                                break;
        case E_ABORT:           WIN::LoadString( m_hInst, IDS_UNKNOWN_ERROR, sError, MAX_STR_LENGTH );
                                break;
        case ERROR_INVALID_PARAMETER:   // 87
                                WIN::LoadString( m_hInst, IDS_INVALID_PARAM, sTmp, MAX_STR_LENGTH );
                                StringCchPrintf( sError, MAX_STR_LENGTH, sTmp, m_pErrorText );
                                break;

        case ERROR_SHOW_USAGE:      // - 2
                                nMsgType = MB_OK | MB_ICONINFORMATION;
                                WIN::LoadString( m_hInst, IDS_USAGE, sError, MAX_STR_LENGTH );
                                break;

        default:                WIN::LoadString( m_hInst, IDS_UNKNOWN_ERROR, sError, MAX_STR_LENGTH );
                                break;
    }

    if ( sError[0] )
    {
        if ( !m_bQuiet )
        {
            ConvertNewline( sError );
            WIN::MessageBox( NULL, sError, m_pAppTitle, nMsgType );
        }

        Log( TEXT( "ERROR: %s\r\n" ), sError );
    }
}

//--------------------------------------------------------------------------
long SetupAppX::GetLanguageID( long nIndex ) const
{
    if ( nIndex >=0 && nIndex < m_nLanguageCount )
        return m_ppLanguageList[ nIndex ]->m_nLanguageID;
    else
        return 0;
}

//--------------------------------------------------------------------------
void SetupAppX::GetLanguageName( long nLanguage, LPTSTR sName ) const
{
    switch ( nLanguage )
    {
        case 1028: WIN::LoadString( m_hInst, IDS_LANGUAGE_ZH_TW, sName, MAX_LANGUAGE_LEN ); break;
        case 1029: WIN::LoadString( m_hInst, IDS_LANGUAGE_CS,    sName, MAX_LANGUAGE_LEN ); break;
        case 1030: WIN::LoadString( m_hInst, IDS_LANGUAGE_DA,    sName, MAX_LANGUAGE_LEN ); break;
        case 1031: WIN::LoadString( m_hInst, IDS_LANGUAGE_DE_DE, sName, MAX_LANGUAGE_LEN ); break;
        case 1032: WIN::LoadString( m_hInst, IDS_LANGUAGE_EL,    sName, MAX_LANGUAGE_LEN ); break;
        case 1033: WIN::LoadString( m_hInst, IDS_LANGUAGE_EN_US, sName, MAX_LANGUAGE_LEN ); break;
        case 1034: WIN::LoadString( m_hInst, IDS_LANGUAGE_ES,    sName, MAX_LANGUAGE_LEN ); break;
        case 1035: WIN::LoadString( m_hInst, IDS_LANGUAGE_FI,    sName, MAX_LANGUAGE_LEN ); break;
        case 1036: WIN::LoadString( m_hInst, IDS_LANGUAGE_FR_FR, sName, MAX_LANGUAGE_LEN ); break;
        case 1037: WIN::LoadString( m_hInst, IDS_LANGUAGE_HE,    sName, MAX_LANGUAGE_LEN ); break;
        case 1038: WIN::LoadString( m_hInst, IDS_LANGUAGE_HU,    sName, MAX_LANGUAGE_LEN ); break;
        case 1040: WIN::LoadString( m_hInst, IDS_LANGUAGE_IT_IT, sName, MAX_LANGUAGE_LEN ); break;
        case 1041: WIN::LoadString( m_hInst, IDS_LANGUAGE_JA,    sName, MAX_LANGUAGE_LEN ); break;
        case 1042: WIN::LoadString( m_hInst, IDS_LANGUAGE_KO,    sName, MAX_LANGUAGE_LEN ); break;
        case 1043: WIN::LoadString( m_hInst, IDS_LANGUAGE_NL_NL, sName, MAX_LANGUAGE_LEN ); break;
        case 1044: WIN::LoadString( m_hInst, IDS_LANGUAGE_NO_NO, sName, MAX_LANGUAGE_LEN ); break;
        case 1045: WIN::LoadString( m_hInst, IDS_LANGUAGE_PL,    sName, MAX_LANGUAGE_LEN ); break;
        case 1046: WIN::LoadString( m_hInst, IDS_LANGUAGE_PT_BR, sName, MAX_LANGUAGE_LEN ); break;
        case 1049: WIN::LoadString( m_hInst, IDS_LANGUAGE_RU,    sName, MAX_LANGUAGE_LEN ); break;
        case 1051: WIN::LoadString( m_hInst, IDS_LANGUAGE_SK,    sName, MAX_LANGUAGE_LEN ); break;
        case 1053: WIN::LoadString( m_hInst, IDS_LANGUAGE_SV_SE, sName, MAX_LANGUAGE_LEN ); break;
        case 1054: WIN::LoadString( m_hInst, IDS_LANGUAGE_TH,    sName, MAX_LANGUAGE_LEN ); break;
        case 1055: WIN::LoadString( m_hInst, IDS_LANGUAGE_TR,    sName, MAX_LANGUAGE_LEN ); break;
        case 1061: WIN::LoadString( m_hInst, IDS_LANGUAGE_ET,    sName, MAX_LANGUAGE_LEN ); break;
        case 2052: WIN::LoadString( m_hInst, IDS_LANGUAGE_ZH_CN, sName, MAX_LANGUAGE_LEN ); break;
        case 2070: WIN::LoadString( m_hInst, IDS_LANGUAGE_PT_PT, sName, MAX_LANGUAGE_LEN ); break;

        default:
            {
                TCHAR sTmp[ MAX_LANGUAGE_LEN ] = {0};

                WIN::LoadString( m_hInst, IDS_UNKNOWN_LANG, sTmp, MAX_LANGUAGE_LEN );
                StringCchPrintf( sName, MAX_LANGUAGE_LEN, sTmp, nLanguage );
            }
    }
}

//--------------------------------------------------------------------------
boolean SetupAppX::CheckVersion()
{
    boolean bRet = true;
    boolean bNeedUpdate = true;
    HMODULE hMsi = LoadMsiLibrary();

    Log( TEXT( " Looking for installed MSI with version >= %s\r\n" ), m_pReqVersion );

    if ( !hMsi )
    {
        Log( TEXT( "Warning: No MSI found, update needed!\r\n" ) );
    }
    else
    {
        PFnDllGetVersion pDllGetVersion = (PFnDllGetVersion) GetProcAddress( hMsi, MSIAPI_DllGetVersion );

        if ( pDllGetVersion )
        {
            DLLVERSIONINFO aInfo;

            aInfo.cbSize = sizeof( DLLVERSIONINFO );
            if ( NOERROR == pDllGetVersion( &aInfo ) )
            {
                TCHAR pMsiVersion[ VERSION_SIZE ];
                StringCchPrintf( pMsiVersion, VERSION_SIZE, TEXT("%d.%d.%4d"),
                                 aInfo.dwMajorVersion,
                                 aInfo.dwMinorVersion,
                                 aInfo.dwBuildNumber );
                if ( _tcsncmp( pMsiVersion, m_pReqVersion, _tcslen( pMsiVersion ) ) < 0 )
                {
                    Log( TEXT( "Warning: Old MSI version found <%s>, update needed!\r\n" ), pMsiVersion );
                }
                else
                {
                    Log( TEXT( " Found MSI version <%s>, no update needed\r\n" ), pMsiVersion );
                    bNeedUpdate = false;
                }
            }
        }

        FreeLibrary( hMsi );
    }

    if ( bNeedUpdate )
    {
        LPTSTR  pInstaller = 0;

        if ( IsWin9x() )
            bRet = GetPathToFile( m_pInstMsiA, &pInstaller );
        else
            bRet = GetPathToFile( m_pInstMsiW, &pInstaller );

        if ( bRet )
            bRet = InstallMsi( pInstaller );
        else
            Log( TEXT( "ERROR: Could not find InstMsiA/InstMsiW!\r\n" ) );

        if ( bRet && IsWin9x() && ( GetMinorVersion() <= 10 ) )
            SetRebootNeeded( true );

        if ( pInstaller ) delete [] pInstaller;
    }

    return bRet;
}

//--------------------------------------------------------------------------
boolean SetupAppX::InstallMsi( LPCTSTR pInstaller )
{
    if ( ! IsAdmin() )
    {
        Log( TEXT( "Error: need admin rights to update/install MSI!\r\n" ) );
        SetError( ERROR_DS_INSUFF_ACCESS_RIGHTS );
        return false;
    }

    if ( ! m_bQuiet )
    {
        TCHAR sUserPrompt[ MAX_STR_LENGTH ] = {0};
        WIN::LoadString( m_hInst, IDS_ALLOW_MSI_UPDATE, sUserPrompt, MAX_STR_LENGTH );
        ConvertNewline( sUserPrompt );

        if ( IDYES != WIN::MessageBox( NULL, sUserPrompt, m_pAppTitle, MB_YESNO | MB_ICONQUESTION ) )
        {
            SetError( ERROR_INSTALL_USEREXIT );
            Log( TEXT( "Error: User canceled update/installation of new MSI!\r\n" ) );
            return false;
        }
    }

    STARTUPINFO         aSUI;
    PROCESS_INFORMATION aPI;

    Log( TEXT( " Will install <%s>\r\n" ), pInstaller );

    ZeroMemory( (void*)&aPI, sizeof( PROCESS_INFORMATION ) );
    ZeroMemory( (void*)&aSUI, sizeof( STARTUPINFO ) );

    aSUI.cb          = sizeof(STARTUPINFO);
    aSUI.dwFlags     = STARTF_USESHOWWINDOW;
    aSUI.wShowWindow = SW_SHOW;

    DWORD  nCmdLineLength = lstrlen( pInstaller ) + lstrlen( sDelayReboot ) + 3;

    if ( m_bQuiet )
        nCmdLineLength += lstrlen( sMsiQuiet );

    TCHAR *sCmdLine = new TCHAR[ nCmdLineLength ];

    if ( FAILED( StringCchCopy( sCmdLine, nCmdLineLength, TEXT("\"")) ) ||
         FAILED( StringCchCat(  sCmdLine, nCmdLineLength, pInstaller) ) ||
         FAILED( StringCchCat(  sCmdLine, nCmdLineLength, TEXT("\"")) ) ||
         FAILED( StringCchCat(  sCmdLine, nCmdLineLength, sDelayReboot) ) ||
         ( m_bQuiet && FAILED( StringCchCat( sCmdLine, nCmdLineLength, sMsiQuiet ) ) ) )
    {
        Log( TEXT( "ERROR: Could not create command line for updating MSI.\r\n" ) );
        delete [] sCmdLine;
        SetError( ERROR_INSTALL_FAILURE );
        return false;
    }

    if ( !WIN::CreateProcess( NULL, sCmdLine, NULL, NULL, FALSE,
                              CREATE_DEFAULT_ERROR_MODE, NULL, NULL,
                              &aSUI, &aPI ) )
    {
        Log( TEXT( "ERROR: Could not create process %s.\r\n" ), sCmdLine );
        SetError( WIN::GetLastError() );
        delete [] sCmdLine;
        return false;
    }

    DWORD nResult = WaitForProcess( aPI.hProcess );

    if( ERROR_SUCCESS != nResult )
    {
        Log( TEXT( "ERROR: While waiting for %s.\r\n" ), sCmdLine );
        delete [] sCmdLine;
        SetError( nResult );
        return false;
    }

    GetExitCodeProcess( aPI.hProcess, &nResult );
    CloseHandle( aPI.hProcess );

    if ( nResult != ERROR_SUCCESS )
    {
        TCHAR sBuf[80];
        StringCchPrintf( sBuf, 80, TEXT("Warning: Installation returned %u.\r\n"), nResult );
        Log( sBuf );
    }
    else
        Log( TEXT( " Installation of new version completed successfully.\r\n" ) );

    delete [] sCmdLine;

    return true;
}

//--------------------------------------------------------------------------
boolean SetupAppX::IsTerminalServerInstalled() const
{
    boolean bIsTerminalServer = false;

    const TCHAR sSearchStr[]   = TEXT("Terminal Server");
    const TCHAR sKey[]         = TEXT("System\\CurrentControlSet\\Control\\ProductOptions");
    const TCHAR sValue[]       = TEXT("ProductSuite");

    DWORD dwSize = 0;
    HKEY  hKey = 0;
    DWORD dwType = 0;

    if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, sKey, 0, KEY_READ, &hKey ) &&
         ERROR_SUCCESS == RegQueryValueEx( hKey, sValue, NULL, &dwType, NULL, &dwSize ) &&
         dwSize > 0 &&
         REG_MULTI_SZ == dwType )
    {
        TCHAR* sSuiteList = new TCHAR[ (dwSize*sizeof(byte)/sizeof(TCHAR)) + 1 ];

        ZeroMemory(sSuiteList, dwSize);

        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, sValue, NULL, &dwType, (LPBYTE)sSuiteList, &dwSize) )
        {
            DWORD nMulti = 0;
            DWORD nSrch  = lstrlen( sSearchStr );
            const TCHAR *sSubString = sSuiteList;

            while (*sSubString)
            {
                nMulti = lstrlen( sSubString );
                if ( nMulti == nSrch && 0 == lstrcmp( sSearchStr, sSubString ) )
                {
                    bIsTerminalServer = true;
                    break;
                }

                sSubString += (nMulti + 1);
            }
        }
        delete [] sSuiteList;
    }

    if ( hKey )
        RegCloseKey( hKey );

    return bIsTerminalServer;
}

//--------------------------------------------------------------------------
boolean SetupAppX::AlreadyRunning() const
{
    if ( m_bIgnoreAlreadyRunning )
    {
        Log( TEXT("Ignoring already running MSI instance!\r\n") );
        return false;
    }

    const TCHAR *sMutexName    = NULL;
    const TCHAR sGUniqueName[] = TEXT( "Global\\_MSISETUP_{EA8130C1-8D3D-4338-9309-1A52D530D846}" );
    const TCHAR sUniqueName[]  = TEXT( "_MSISETUP_{EA8130C1-8D3D-4338-9309-1A52D530D846}" );

    if ( IsWin9x() )
        sMutexName = sUniqueName;
    else if ( ( GetOSVersion() < 5 ) && ! IsTerminalServerInstalled() )
        sMutexName = sUniqueName;
    else
        sMutexName = sGUniqueName;

    HANDLE hMutex = 0;

    hMutex = WIN::CreateMutex( NULL, FALSE, sMutexName );

    if ( !hMutex || ERROR_ALREADY_EXISTS == WIN::GetLastError() )
    {
        if ( !hMutex )
            Log( TEXT( "ERROR: AlreadyRunning() could not create mutex!\r\n" ) );
        else
            Log( TEXT( "ERROR: There's already a setup running!\r\n" ) );

        return true;
    }
    Log( TEXT( " No running Setup found\r\n" ) );

    return false;
}

//--------------------------------------------------------------------------
DWORD SetupAppX::WaitForProcess( HANDLE hHandle )
{
    DWORD nResult = NOERROR;
    boolean bLoop = true;

    MSG aMsg;
    ZeroMemory( (void*) &aMsg, sizeof(MSG) );

    while ( bLoop )
    {
        switch ( WIN::MsgWaitForMultipleObjects( 1, &hHandle, false,
                                                 INFINITE, QS_ALLINPUT ) )
        {
            case WAIT_OBJECT_0: bLoop = false;
                break;

            case (WAIT_OBJECT_0 + 1):
            {
                if ( WIN::PeekMessage( &aMsg, NULL, NULL, NULL, PM_REMOVE ) )
                {
                    WIN::TranslateMessage( &aMsg );
                    WIN::DispatchMessage( &aMsg );
                }
                break;
            }

            default:
            {
                nResult = WIN::GetLastError();
                bLoop = false;
            }
        }
    }

    return nResult;
}

//--------------------------------------------------------------------------
void SetupAppX::Log( LPCTSTR pMessage, LPCTSTR pText ) const
{
    if ( m_pLogFile )
    {
        static boolean bInit = false;

        if ( !bInit )
        {
            bInit = true;
            if ( ! IsWin9x() )
                _ftprintf( m_pLogFile, TEXT("%c"), 0xfeff );

            _tsetlocale( LC_ALL, TEXT("") );
            _ftprintf( m_pLogFile, TEXT("\nCodepage=%s\nMultiByte Codepage=[%d]\n"),
                                   _tsetlocale( LC_ALL, NULL ), _getmbcp() );
        }
        if ( pText )
            _ftprintf( m_pLogFile, pMessage, pText );
        else
            _ftprintf( m_pLogFile, pMessage );

        fflush( m_pLogFile );
    }
}

//--------------------------------------------------------------------------
DWORD SetupAppX::GetNextArgument( LPCTSTR pStr, LPTSTR *pArg, LPTSTR *pNext,
                                  boolean bStripQuotes )
{
    boolean bInQuotes = false;
    boolean bFoundArgEnd = false;
    LPCTSTR pChar = pStr;
    LPCTSTR pFirst = NULL;

    if ( NULL == pChar )
        return ERROR_NO_MORE_ITEMS;

    while ( ' ' == (*pChar) || '\t' == (*pChar) )
        pChar = CharNext( pChar );

    if ( '\0' == (*pChar) )
        return ERROR_NO_MORE_ITEMS;

    int nCount = 1;
    pFirst = pChar;

    while ( ! bFoundArgEnd )
    {
        if ( '\0' == (*pChar) )
            bFoundArgEnd = true;
        else if ( !bInQuotes && ' ' == (*pChar) )
            bFoundArgEnd = true;
        else if ( !bInQuotes && '\t' == (*pChar) )
            bFoundArgEnd = true;
        else
        {
            if ( '\"' == (*pChar) )
            {
                bInQuotes = !bInQuotes;
                if ( bStripQuotes )
                {
                    if ( pChar == pFirst )
                        pFirst = CharNext( pFirst );
                    nCount -= 1;
                }
            }

            pChar = CharNext( pChar );
            nCount += 1;
        }
    }

    if ( pArg )
    {
        *pArg = new TCHAR[ nCount ];
        StringCchCopyN ( *pArg, nCount, pFirst, nCount-1 );
    }

    if ( pNext )
        *pNext = CharNext( pChar );

    return ERROR_SUCCESS;
}

//--------------------------------------------------------------------------
boolean SetupAppX::GetCmdLineParameters( LPTSTR *pCmdLine )
{
    int    nRet   = ERROR_SUCCESS;
    LPTSTR pStart = NULL;
    LPTSTR pNext  = NULL;

    if ( GetNextArgument( *pCmdLine, NULL, &pNext ) != ERROR_SUCCESS )
    {
        SetError( ERROR_NO_MORE_ITEMS );
        return false;
    }

    int    nSize = lstrlen( *pCmdLine ) + 2;
    TCHAR *pNewCmdLine = new TCHAR[ nSize ];
    pNewCmdLine[0] = '\0';

    while ( GetNextArgument( pNext, &pStart, &pNext ) == ERROR_SUCCESS )
    {
        boolean bDeleteStart = true;

        if ( (*pStart) == '/' || (*pStart) == '-' )
        {
            LPTSTR pSub = CharNext( pStart );
            if ( (*pSub) == 'l' || (*pSub) == 'L' )
            {
                pSub = CharNext( pSub );
                if ( (*pSub) == 'a' || (*pSub) == 'A' )
                {   // --- handle the lang parameter ---
                    LPTSTR pLanguage = NULL;
                    LPTSTR pLastChar;
                    if ( GetNextArgument( pNext, &pLanguage, &pNext, true ) != ERROR_SUCCESS )
                    {
                        StringCchCopy( m_pErrorText, MAX_STR_LENGTH, pStart );
                        nRet = ERROR_INVALID_PARAMETER;
                        break;
                    }

                    m_nLanguageID = _tcstol( pLanguage, &pLastChar, 10 );
                    delete [] pLanguage;
                }
                else
                {   // --- handle the l(og) parameter ---
                    boolean bAppend = false;
                    LPTSTR  pFileName = NULL;

                    while ( *pSub )
                    {
                        if ( *pSub == '+' )
                        {
                            bAppend = true;
                            break;
                        }
                        pSub = CharNext( pSub );
                    }

                    if ( GetNextArgument( pNext, &pFileName, &pNext, true ) != ERROR_SUCCESS )
                    {
                        StringCchCopy( m_pErrorText, MAX_STR_LENGTH, pStart );
                        nRet = ERROR_INVALID_PARAMETER;
                        break;
                    }

                    if ( FAILED( StringCchCat( pNewCmdLine, nSize, pStart ) ) )
                    {
                        nRet = ERROR_OUTOFMEMORY;
                        break;
                    }
                    // we need to append a '+' otherwise msiexec would overwrite our log file
                    if ( !bAppend && FAILED( StringCchCat( pNewCmdLine, nSize, TEXT( "+" ) ) ) )
                    {
                        nRet = ERROR_OUTOFMEMORY;
                        break;
                    }
                    if ( FAILED( StringCchCat( pNewCmdLine, nSize, TEXT( " \"" ) ) ) ||
                        FAILED( StringCchCat( pNewCmdLine, nSize, pFileName ) ) ||
                        FAILED( StringCchCat( pNewCmdLine, nSize, TEXT( "\" " ) ) ) )
                    {
                        nRet = ERROR_OUTOFMEMORY;
                        break;
                    }

                    if ( bAppend )
                        m_pLogFile = _tfopen( pFileName, TEXT( "ab" ) );
                    else
                        m_pLogFile = _tfopen( pFileName, TEXT( "wb" ) );

                    delete [] pFileName;
                }
            }
            else if ( (*pSub) == 'q' || (*pSub) == 'Q' )
            {   // --- Handle quiet file parameter ---
                pSub = CharNext( pSub );
                if ( ! (*pSub) || (*pSub) == 'n' || (*pSub) == 'N' )
                    m_bQuiet = true;

                if ( FAILED( StringCchCat( pNewCmdLine, nSize, pStart ) ) ||
                     FAILED( StringCchCat( pNewCmdLine, nSize, TEXT( " " ) ) ) )
                {
                    nRet = ERROR_OUTOFMEMORY;
                    break;
                }
            }
            else if ( _tcsnicmp( pSub, PARAM_RUNNING, _tcslen( PARAM_RUNNING ) ) == 0 )
            {
                m_bIgnoreAlreadyRunning = true;
            }
            else if ( (*pSub) == 'i' || (*pSub) == 'I' || (*pSub) == 'f' || (*pSub) == 'F' ||
                      (*pSub) == 'p' || (*pSub) == 'P' || (*pSub) == 'x' || (*pSub) == 'X' ||
                      (*pSub) == 'y' || (*pSub) == 'Y' || (*pSub) == 'z' || (*pSub) == 'Z' )
            {
                StringCchCopy( m_pErrorText, MAX_STR_LENGTH, pStart );
                nRet = ERROR_INVALID_PARAMETER;
                break;
            }
            else if ( (*pSub) == 'a' || (*pSub) == 'A' )
            {   // --- Handle Adminstrative Installation ---
                m_bAdministrative = true;
            }
            else if ( (*pSub) == 'j' || (*pSub) == 'J' )
            {   // --- Handle Adminstrative Installation ---
                m_pAdvertise = pStart;
                m_bQuiet     = true;
                bDeleteStart = false;
            }
            else if ( (*pSub) == '?' || (*pSub) == 'h' || (*pSub) == 'H' )
            {   // --- Handle Show Usage ---
                nRet = ERROR_SHOW_USAGE;
                break;
            }
            else
            {
                if ( FAILED( StringCchCat( pNewCmdLine, nSize, pStart ) ) ||
                     FAILED( StringCchCat( pNewCmdLine, nSize, TEXT( " " ) ) ) )
                {
                    nRet = ERROR_OUTOFMEMORY;
                    break;
                }
            }
        }
        else
        {
            if ( FAILED( StringCchCat( pNewCmdLine, nSize, pStart ) ) ||
                 FAILED( StringCchCat( pNewCmdLine, nSize, TEXT( " " ) ) ) )
            {
                nRet = ERROR_OUTOFMEMORY;
                break;
            }
        }

        if ( bDeleteStart ) delete [] pStart;
        pStart = NULL;
    }

    if ( pStart ) delete [] pStart;

    *pCmdLine = pNewCmdLine;

    if ( nRet != ERROR_SUCCESS )
    {
        SetError( nRet );
        return false;
    }
    else
        return true;;
}

//--------------------------------------------------------------------------
boolean SetupAppX::IsAdmin()
{
    if ( IsWin9x() )
        return true;

    PSID aPsidAdmin;
    SID_IDENTIFIER_AUTHORITY aAuthority = SECURITY_NT_AUTHORITY;

    if ( !AllocateAndInitializeSid( &aAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                    &aPsidAdmin ) )
        return false;

    BOOL bIsAdmin = FALSE;

    if ( GetOSVersion() >= 5 )
    {
        HMODULE hAdvapi32 = LoadLibrary( ADVAPI32_DLL );

        if ( !hAdvapi32 )
            bIsAdmin = FALSE;
        else
        {
            PFnCheckTokenMembership pfnCheckTokenMembership = (PFnCheckTokenMembership) GetProcAddress( hAdvapi32, ADVAPI32API_CheckTokenMembership);
            if ( !pfnCheckTokenMembership || !pfnCheckTokenMembership( NULL, aPsidAdmin, &bIsAdmin ) )
                bIsAdmin = FALSE;
        }
        FreeLibrary( hAdvapi32 );
    }
    else
    {
        // NT4, check groups of user
        HANDLE hAccessToken = 0;
        UCHAR *szInfoBuffer = new UCHAR[ 1024 ]; // may need to resize if TokenInfo too big
        DWORD dwInfoBufferSize = 1024;
        DWORD dwRetInfoBufferSize = 0;
        UINT i=0;

        if ( WIN::OpenProcessToken( WIN::GetCurrentProcess(), TOKEN_READ, &hAccessToken ) )
        {
            bool bSuccess = false;
            bSuccess = WIN::GetTokenInformation( hAccessToken, TokenGroups,
                                                 szInfoBuffer, dwInfoBufferSize,
                                                 &dwRetInfoBufferSize ) == TRUE;

            if( dwRetInfoBufferSize > dwInfoBufferSize )
            {
                delete [] szInfoBuffer;
                szInfoBuffer = new UCHAR[ dwRetInfoBufferSize ];
                dwInfoBufferSize = dwRetInfoBufferSize;
                bSuccess = WIN::GetTokenInformation( hAccessToken, TokenGroups,
                                                     szInfoBuffer, dwInfoBufferSize,
                                                     &dwRetInfoBufferSize ) == TRUE;
            }

            WIN::CloseHandle( hAccessToken );

            if ( bSuccess )
            {
                PTOKEN_GROUPS pGroups = (PTOKEN_GROUPS)(UCHAR*) szInfoBuffer;
                for( i=0; i<pGroups->GroupCount; i++ )
                {
                    if( WIN::EqualSid( aPsidAdmin, pGroups->Groups[i].Sid ) )
                    {
                        bIsAdmin = TRUE;
                        break;
                    }
                }
            }

            delete [] szInfoBuffer;
        }
    }

    WIN::FreeSid( aPsidAdmin );

    return bIsAdmin ? true : false;
}

//--------------------------------------------------------------------------
LPTSTR SetupAppX::CopyIniFile( LPCTSTR pIniFile )
{
    m_pTmpName = _ttempnam( TEXT( "C:\\" ), TEXT( "Setup" ) );

    if ( !m_pTmpName )
    {
        Log( TEXT( "ERROR: Could not create temp file\n" ) );
        return NULL;
    }

    FILE *pOut  = _tfopen( m_pTmpName, TEXT( "wb" ) );
    FILE *pIn   = _tfopen( pIniFile, TEXT( "rb" ) );

    if ( pOut && pIn )
    {
        size_t  nRead, nWritten;
        BYTE    pBuf[1024];

        nRead = fread( pBuf, sizeof( BYTE ), 1024, pIn );
        while ( nRead && !ferror( pIn ) )
        {
            nWritten = fwrite( pBuf, sizeof( BYTE ), nRead, pOut );
            if ( nWritten != nRead )
            {
                Log( TEXT( "ERROR: Could not write all bytes to temp file\n" ) );
                break;
            }
            nRead = fread( pBuf, sizeof( BYTE ), 1024, pIn );
        }
    }

    if ( pOut ) fclose( pOut );
    if ( pIn ) fclose( pIn );

    return m_pTmpName;
}

//--------------------------------------------------------------------------
void SetupAppX::ConvertNewline( LPTSTR pText ) const
{
    int i=0;

    while ( pText[i] != 0 )
    {
        if ( ( pText[i] == '\\' ) && ( pText[i+1] == 'n' ) )
        {
            pText[i] = 0x0d;
            pText[i+1] = 0x0a;
            i+=2;
        }
        else
            i+=1;
    }
}

//--------------------------------------------------------------------------
LPTSTR SetupAppX::SetProdToAppTitle( LPCTSTR pProdName )
{
    if ( !pProdName ) return m_pAppTitle;

    LPTSTR pAppProdTitle = new TCHAR[ MAX_STR_CAPTION ];
           pAppProdTitle[0]  = '\0';

    WIN::LoadString( m_hInst, IDS_APP_PROD_TITLE, pAppProdTitle, MAX_STR_CAPTION );

    int nAppLen = lstrlen( pAppProdTitle );
    int nProdLen = lstrlen( pProdName );

    if ( ( nAppLen == 0 ) || ( nProdLen == 0 ) )
    {
        delete [] pAppProdTitle;
        return m_pAppTitle;
    }

    int nLen = nAppLen + nProdLen + 3;

    if ( nLen > STRSAFE_MAX_CCH ) return m_pAppTitle;

    LPTSTR pIndex = _tcsstr( pAppProdTitle, PRODUCT_NAME_VAR );

    if ( pIndex )
    {
        int nOffset = pIndex - pAppProdTitle;
        int nVarLen = lstrlen( PRODUCT_NAME_VAR );

        LPTSTR pNewTitle = new TCHAR[ nLen ];
        pNewTitle[0] = '\0';

        if ( nOffset > 0 )
        {
            StringCchCopyN( pNewTitle, nLen, pAppProdTitle, nOffset );
        }

        StringCchCat( pNewTitle, nLen, pProdName );

        if ( nOffset + nVarLen < nAppLen )
        {
            StringCchCat( pNewTitle, nLen, pIndex + nVarLen );
        }

        delete [] m_pAppTitle;
        m_pAppTitle = pNewTitle;
    }

    delete [] pAppProdTitle;

    return m_pAppTitle;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
LanguageDataX::LanguageDataX( LPTSTR pData )
{
    m_nLanguageID = 0;
    m_pTransform = NULL;

    LPTSTR pLastChar;

    m_nLanguageID = _tcstol( pData, &pLastChar, 10 );

    if ( *pLastChar == ',' )
    {
        pLastChar += 1;
        int nLen = lstrlen( pLastChar ) + 1;
        m_pTransform = new TCHAR [ nLen ];
        StringCchCopy( m_pTransform, nLen, pLastChar );
    }
}

//--------------------------------------------------------------------------
LanguageDataX::~LanguageDataX()
{
    if ( m_pTransform ) delete [] m_pTransform;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
SetupApp* Create_SetupAppX()
{
    return new SetupAppX;
}

//--------------------------------------------------------------------------
