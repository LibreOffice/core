/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"
#define UNICODE
#define _UNICODE

#include <cstddef>

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <shellapi.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <tchar.h>

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <systools/win32/uwinapi.h>

#include "rtl/string.h"
#include <sal/macros.h>

#include "../../../source/inc/exithelper.hxx"
#include "../extendloaderenvironment.hxx"

#define PIPE_PREFIX                 TEXT("\\\\.\\pipe\\OSL_PIPE_")
#define PIPE_POSTFIX                TEXT("_SingleOfficeIPC_")
#define PIPE_TERMINATION_SEQUENCE   "InternalIPC::ProcessingDone"

BOOL WINAPI ConvertSidToStringSid( PSID pSid, LPTSTR* StringSid )
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;

    // Validate the binary SID.

    if(!IsValidSid(pSid)) return FALSE;

    // Get the identifier authority value from the SID.

    psia = GetSidIdentifierAuthority(pSid);

    // Get the number of subauthorities in the SID.

    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

    // Compute the buffer length.
    // S-SID_REVISION- + IdentifierAuthority- + subauthorities- + NULL

    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

    *StringSid = (LPTSTR)LocalAlloc( LMEM_FIXED, dwSidSize );

    // Add 'S' prefix and revision number to the string.

    dwSidSize=wsprintf(*StringSid, TEXT("S-%lu-"), dwSidRev );

    // Add a SID identifier authority to the string.

    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
    {
        dwSidSize+=wsprintf(*StringSid + lstrlen(*StringSid),
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize+=wsprintf(*StringSid + lstrlen(*StringSid),
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

    // Add SID subauthorities to the string.
    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        dwSidSize+=wsprintf(*StringSid + dwSidSize, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

    return TRUE;
}


//---------------------------------------------------------------------------

static LPTSTR   *GetCommandArgs( int *pArgc )
{
#ifdef UNICODE
    return CommandLineToArgvW( GetCommandLineW(), pArgc );
#else
    *pArgc = __argc;
    return __argv;
#endif
}

//---------------------------------------------------------------------------

namespace {

bool writeArgument(HANDLE pipe, char prefix, WCHAR const * argument) {
    CHAR szBuffer[4096];
    int n = WideCharToMultiByte(
        CP_UTF8, 0, argument, -1, szBuffer, sizeof (szBuffer), NULL, NULL);
    char b[1 + 2 * ((sizeof szBuffer) - 1)]; // hopefully does not overflow
    b[0] = prefix;
    char * p = b + 1;
    for (int i = 0; i < n - 1; ++i) { // cannot underflow (n >= 0)
        char c = szBuffer[i];
        switch (c) {
        case '\0':
            *p++ = '\\';
            *p++ = '0';
            break;
        case ',':
            *p++ = '\\';
            *p++ = ',';
            break;
        case '\\':
            *p++ = '\\';
            *p++ = '\\';
            break;
        default:
            *p++ = c;
            break;
        }
    }
    DWORD w;
    return WriteFile(pipe, b, p - b, &w, NULL);
}

}

#ifdef __MINGW32__
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int )
#else
int WINAPI _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )
#endif
{
    TCHAR               szTargetFileName[MAX_PATH] = TEXT("");
    TCHAR               szIniDirectory[MAX_PATH];
    TCHAR               szPerfTuneIniFile[MAX_PATH] = TEXT("");
    STARTUPINFO         aStartupInfo;

    desktop_win32::extendLoaderEnvironment(szTargetFileName, szIniDirectory);

    ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
    aStartupInfo.cb = sizeof(aStartupInfo);

    GetStartupInfo( &aStartupInfo );
    // Get image path with same name but with .bin extension

    TCHAR               szModuleFileName[MAX_PATH];

    GetModuleFileName( NULL, szModuleFileName, MAX_PATH );
    _TCHAR  *lpLastSlash = _tcsrchr( szModuleFileName, '\\' );
    if ( lpLastSlash )
    {
        size_t len = lpLastSlash - szModuleFileName + 1;
        _tcsncpy( szPerfTuneIniFile, szModuleFileName, len );
        _tcsncpy( szPerfTuneIniFile + len, _T("perftune.ini"), SAL_N_ELEMENTS(szPerfTuneIniFile) - len );
    }

    // Create process with same command line, environment and stdio handles which
    // are directed to the created pipes

    DWORD   dwExitCode = (DWORD)-1;

    BOOL    fSuccess = FALSE;
    LPTSTR  lpCommandLine = NULL;
    int argc = 0;
    LPTSTR * argv = NULL;
    bool bFirst = true;
    WCHAR cwd[MAX_PATH];
    DWORD cwdLen = GetCurrentDirectoryW(MAX_PATH, cwd);
    if (cwdLen >= MAX_PATH) {
        cwdLen = 0;
    }

    do
    {
        TCHAR   szKey[32];

        GetPrivateProfileString(
            TEXT("PerformanceTuning"),
            TEXT("FastPipeCommunication"),
            TEXT("0"),
            szKey,
            SAL_N_ELEMENTS(szKey),
            szPerfTuneIniFile
            );

        if ( 0 == _tcscmp( szKey, TEXT("1") ) )
        {
            HANDLE  hProcessToken;

            if ( OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hProcessToken ) )
            {
                TCHAR   szPipeName[4096];


                DWORD   dwTokenLength = 0;


                fSuccess = GetTokenInformation( hProcessToken, TokenUser, NULL, dwTokenLength, &dwTokenLength );

                PVOID   pTokenInfo = _alloca(dwTokenLength);
                fSuccess = GetTokenInformation( hProcessToken, TokenUser, pTokenInfo, dwTokenLength, &dwTokenLength );
                CloseHandle( hProcessToken );

                PSID pSid = ((PTOKEN_USER)pTokenInfo)->User.Sid;
                LPTSTR  szUserIdent = NULL;
                TCHAR   szSUPD[11] = TEXT("0");

                fSuccess = ConvertSidToStringSid( pSid, &szUserIdent );

                _tcsncpy( szPipeName, PIPE_PREFIX, SAL_N_ELEMENTS(szPipeName) );
                _tcsncat( szPipeName, szUserIdent, SAL_N_ELEMENTS(szPipeName) - _tcslen(szPipeName) - 1 );
                _tcsncat( szPipeName, PIPE_POSTFIX, SAL_N_ELEMENTS(szPipeName) - _tcslen(szPipeName) - 1 );
                _tcsncat( szPipeName, _ultot( SUPD, szSUPD, 10), SAL_N_ELEMENTS(szPipeName) - _tcslen(szPipeName) - 1 );

                LocalFree( szUserIdent );

                HANDLE  hPipe = CreateFile(
                                    szPipeName,
                                    GENERIC_READ|GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

                if ( INVALID_HANDLE_VALUE != hPipe )
                {
                    DWORD   dwBytesWritten;
                    int argc = 0;
                    LPWSTR  *argv = CommandLineToArgvW( GetCommandLine(), &argc );

                    fSuccess = WriteFile( hPipe, RTL_CONSTASCII_STRINGPARAM("InternalIPC::Arguments"), &dwBytesWritten, NULL );
                    if (fSuccess) {
                        if (cwdLen > 0) {
                            fSuccess = writeArgument(hPipe, '2', cwd);
                        } else {
                            fSuccess = WriteFile(
                                hPipe, RTL_CONSTASCII_STRINGPARAM("0"),
                                &dwBytesWritten, NULL);
                        }
                    }
                    for ( int argn = 1; fSuccess && argn < argc; argn++ )
                    {
                        fSuccess = writeArgument(hPipe, ',', argv[argn]);
                    }

                    if ( fSuccess )
                    {
                        fSuccess = WriteFile(  hPipe, "", 1, &dwBytesWritten, NULL );
                        if ( fSuccess )
                        {
                            DWORD   dwBytesRead = 0;
                            char    *pBuffer = (char *)_alloca( sizeof(PIPE_TERMINATION_SEQUENCE) );
                            fSuccess = ReadFile( hPipe, pBuffer, sizeof(PIPE_TERMINATION_SEQUENCE) - 1, &dwBytesRead, NULL );
                            if ( fSuccess )
                            {
                                pBuffer[dwBytesRead] = 0;
                                if ( 0 != strcmp( PIPE_TERMINATION_SEQUENCE, pBuffer ) )
                                    fSuccess = FALSE;
                            }
                        }
                    }

                    CloseHandle( hPipe );

                    return fSuccess ? 0 : -1;
                }

            }
        }

        if ( bFirst ) {
            argv = GetCommandArgs(&argc);
            std::size_t n = wcslen(argv[0]) + 2;
            for (int i = 1; i < argc; ++i) {
                n += wcslen(argv[i]) + 3;
            }
            n += MY_LENGTH(L" \"-env:OOO_CWD=2") + 4 * cwdLen +
                MY_LENGTH(L"\"") + 1;
                // 4 * cwdLen: each char preceded by backslash, each trailing
                // backslash doubled
            lpCommandLine = new WCHAR[n];
        }
        WCHAR * p = desktop_win32::commandLineAppend(
            lpCommandLine, MY_STRING(L"\""));
        p = desktop_win32::commandLineAppend(p, argv[0]);
        for (int i = 1; i < argc; ++i) {
            if (bFirst || ::desktop::ExitHelper::E_NORMAL_RESTART == dwExitCode || wcsncmp(argv[i], MY_STRING(L"-env:")) == 0) {
                p = desktop_win32::commandLineAppend(p, MY_STRING(L"\" \""));
                p = desktop_win32::commandLineAppend(p, argv[i]);
            }
        }

        p = desktop_win32::commandLineAppend(
            p, MY_STRING(L"\" \"-env:OOO_CWD="));
        if (cwdLen == 0) {
            p = desktop_win32::commandLineAppend(p, MY_STRING(L"0"));
        } else {
            p = desktop_win32::commandLineAppend(p, MY_STRING(L"2"));
            p = desktop_win32::commandLineAppendEncoded(p, cwd);
        }
        desktop_win32::commandLineAppend(p, MY_STRING(L"\""));
        bFirst = false;

        TCHAR   szParentProcessId[64]; // This is more than large enough for a 128 bit decimal value
        BOOL    bHeadlessMode( FALSE );

        {
            // Check command line arguments for "-headless" parameter. We only
            // set the environment variable "ATTACHED_PARENT_PROCESSID" for the headless
            // mode as self-destruction of the soffice.bin process can lead to
            // certain side-effects (log-off can result in data-loss, ".lock" is not deleted.
            // See 138244 for more information.
            int     argc;
            LPTSTR  *argv = GetCommandArgs( &argc );

            if ( argc > 1 )
            {
                int n;

                for ( n = 1; n < argc; n++ )
                {
                    if ( 0 == _tcsnicmp( argv[n], _T("-headless"), 9 ) )
                        bHeadlessMode = TRUE;
                }
            }
        }

        if ( _ltot( (long)GetCurrentProcessId(),szParentProcessId, 10 ) && bHeadlessMode )
            SetEnvironmentVariable( TEXT("ATTACHED_PARENT_PROCESSID"), szParentProcessId );

        PROCESS_INFORMATION aProcessInfo;

        fSuccess = CreateProcess(
            szTargetFileName,
            lpCommandLine,
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            szIniDirectory,
            &aStartupInfo,
            &aProcessInfo );

        if ( fSuccess )
        {
            DWORD   dwWaitResult;

            do
            {
                // On Windows XP it seems as the desktop calls WaitForInputIdle after "OpenWidth" so we have to do so
                // as if we where processing any messages

                dwWaitResult = MsgWaitForMultipleObjects( 1, &aProcessInfo.hProcess, FALSE, INFINITE, QS_ALLEVENTS );

                if (  WAIT_OBJECT_0 + 1 == dwWaitResult )
                {
                    MSG msg;

                    PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
                }
            } while ( WAIT_OBJECT_0 + 1 == dwWaitResult );

            dwExitCode = 0;
            GetExitCodeProcess( aProcessInfo.hProcess, &dwExitCode );

            CloseHandle( aProcessInfo.hProcess );
            CloseHandle( aProcessInfo.hThread );
        }
    } while ( false );
    delete[] lpCommandLine;

    return fSuccess ? dwExitCode : -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
