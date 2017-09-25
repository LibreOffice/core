/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include "cppuhelper/findsofficepath.h"
#include "sal/types.h"

#define MY_SIZE(s) (sizeof (s) / sizeof *(s))
#define MY_LENGTH(s) (MY_SIZE(s) - 1)

wchar_t* getPath(void);
wchar_t* createCommandLine( wchar_t const * lpCmdLine );
FILE* getErrorFile( int create );
void writeError( const char* errstr );
void closeErrorFile(void);

/*
 * The main function implements a loader for applications which use UNO.
 *
 * <p>This code runs on the Windows platform only.</p>
 *
 * <p>The main function detects a UNO installation on the system and adds the
 * program directory of the UNO installation to the PATH environment variable.
 * After that, the application process is loaded and started, whereby the
 * new process inherits the environment of the calling process, including
 * the modified PATH environment variable. The application's executable name
 * must be the same as the name of this executable, prefixed by '_'.</p>
 *
 * <p>A UNO installation can be specified by the user by setting the UNO_PATH
 * environment variable to the program directory of the UNO installation.
 * If no installation is specified by the user, the default installation on
 * the system will be taken. The default installation is read from the
 * default value of the key "Software\LibreOffice\UNO\InstallPath" from the
 * root key HKEY_CURRENT_USER in the Windows Registry. If this key is missing,
 * the key is read from the root key HKEY_LOCAL_MACHINE.</p>
 */
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow )
{
    (void) hInstance; /* unused */
    (void) hPrevInstance; /* unused */
    (void) nCmdShow; /* unused */

    /* get the path of the UNO installation */
    wchar_t* path = getPath();

    if ( path != NULL )
    {
        wchar_t cmd[
            MY_LENGTH(L"\"") + MAX_PATH +
            MY_LENGTH(L"\\unoinfo.exe\" c++")];
            /* hopefully does not overflow */
        cmd[0] = L'"';
        wcscpy(cmd + 1, path);
        if (wcschr(cmd + 1, L'"') != NULL) {
            free(path);
            writeError("Error: bad characters in UNO installation path!\n");
            closeErrorFile();
            return 1;
        }
        size_t pathsize = wcslen(cmd);
        wcscpy(
            cmd + pathsize,
            &L"\\unoinfo.exe\" c++"[
                pathsize == 1 || cmd[pathsize - 1] != L'\\' ? 0 : 1]);
        SECURITY_ATTRIBUTES sec;
        sec.nLength = sizeof (SECURITY_ATTRIBUTES);
        sec.lpSecurityDescriptor = NULL;
        sec.bInheritHandle = TRUE;
        HANDLE stdoutRead;
        HANDLE stdoutWrite;
        HANDLE temp;
        if (CreatePipe(&temp, &stdoutWrite, &sec, 0) == 0 ||
            DuplicateHandle(
                GetCurrentProcess(), temp, GetCurrentProcess(), &stdoutRead, 0,
                FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS) == 0)
        {
            free(path);
            writeError("Error: CreatePipe/DuplicateHandle failed!\n");
            closeErrorFile();
            return 1;
        }
        STARTUPINFOW startinfo;
        PROCESS_INFORMATION procinfo;
        memset(&startinfo, 0, sizeof(startinfo));
        startinfo.cb = sizeof(startinfo);
        startinfo.lpDesktop = L"";
        startinfo.dwFlags = STARTF_USESTDHANDLES;
        startinfo.hStdOutput = stdoutWrite;
        BOOL ret = CreateProcessW(
            NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &startinfo, &procinfo);
        if (ret != FALSE) {
            // Release result of GetPath()
            free(path);

            char * buf = NULL;
            char * tmp;
            DWORD n = 1000;
            DWORD k = 0;
            DWORD exitcode;
            CloseHandle(stdoutWrite);
            CloseHandle(procinfo.hThread);
            for (;;) {
                DWORD m;
                tmp = realloc(buf, n);
                if (tmp == NULL) {
                    free(buf);
                    writeError(
                        "Error: out of memory reading unoinfo output!\n");
                    closeErrorFile();
                    return 1;
                }
                buf = tmp;
                if (!ReadFile(stdoutRead, buf + k, n - k, &m, NULL))
                {
                    DWORD err = GetLastError();
                    if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE) {
                        break;
                    }
                    writeError("Error: cannot read unoinfo output!\n");
                    closeErrorFile();
                    return 1;
                }
                if (m == 0) {
                    break;
                }
                k += m;
                if (k >= n) {
                    if (n >= MAXDWORD / 2) {
                        writeError(
                            "Error: out of memory reading unoinfo output!\n");
                        closeErrorFile();
                        return 1;
                    }
                    n *= 2;
                }
            }
            if ((k & 1) == 1) {
                writeError("Error: bad unoinfo output!\n");
                closeErrorFile();
                return 1;
            }
            CloseHandle(stdoutRead);
            if (!GetExitCodeProcess(procinfo.hProcess, &exitcode) ||
                exitcode != 0)
            {
                writeError("Error: executing unoinfo failed!\n");
                closeErrorFile();
                return 1;
            }
            path = (wchar_t*)realloc(buf, k + sizeof(wchar_t));
            if (path == NULL)
            {
                free(buf);
                writeError(
                    "Error: out of memory zero-terminating unoinfo output!\n");
                closeErrorFile();
                return 1;
            }
            path[k / 2] = L'\0';
        } else {
            if (GetLastError() != ERROR_FILE_NOT_FOUND) {
                free(path);
                writeError("Error: calling unoinfo failed!\n");
                closeErrorFile();
                return 1;
            }
            CloseHandle(stdoutRead);
            CloseHandle(stdoutWrite);
        }

        /* get the value of the PATH environment variable */
        const wchar_t* ENVVARNAME = L"PATH";
        const wchar_t* PATHSEPARATOR = L";";
        wchar_t* value = _wgetenv( ENVVARNAME );

        /*
         * add the UNO installation path to the PATH environment variable;
         * note that this only affects the environment variable of the current
         * process, the command processor's environment is not changed
         */
        size_t size = wcslen( ENVVARNAME ) + wcslen( L"=" ) + wcslen( path ) + 1;
        if ( value != NULL )
            size += wcslen( PATHSEPARATOR ) + wcslen( value );
        wchar_t* envstr = (wchar_t*) malloc( size*sizeof(wchar_t) );
        wcscpy( envstr, ENVVARNAME );
        wcscat( envstr, L"=" );
        wcscat( envstr, path );
        if ( value != NULL )
        {
            wcscat( envstr, PATHSEPARATOR );
            wcscat( envstr, value );
        }
        _wputenv( envstr );
        free( envstr );
        free( path );
    }
    else
    {
        writeError( "Warning: no UNO installation found!\n" );
    }

    /* create the command line for the application process */
    wchar_t* cmdline = createCommandLine( lpCmdLine );
    if ( cmdline == NULL )
    {
        writeError( "Error: cannot create command line!\n" );
        closeErrorFile();
        return 1;
    }

    /* create the application process */
    STARTUPINFOW startup_info;
    PROCESS_INFORMATION process_info;
    memset( &startup_info, 0, sizeof(startup_info) );
    startup_info.cb = sizeof(startup_info);
    BOOL bCreate = CreateProcessW( NULL, cmdline, NULL,  NULL, FALSE, 0, NULL, NULL,
        &startup_info, &process_info );
    free( cmdline );
    if ( !bCreate )
    {
        writeError( "Error: cannot create process!\n" );
        closeErrorFile();
        return 1;
    }

    /* close the error file */
    closeErrorFile();

    return 0;
}

/*
 * Gets the path of a UNO installation.
 *
 * @return the installation path or NULL, if no installation was specified or
 *         found, or if an error occurred.
 *         Returned pointer must be released with free()
 */
wchar_t* getPath()
{
    wchar_t* path = cppuhelper_detail_findSofficePath();

    if ( path == NULL )
        writeError( "Warning: getting path from Windows Registry failed!\n" );

    return path;
}

/*
 * Creates the command line for the application process including the absolute
 * path of the executable.
 *
 * <p>The application's executable file name is the name of this executable
 * prefixed by '_'.</p>
 *
 * @param appendix specifies the command line for the application excluding
 *                 the executable name
 *
 * @return the command line for the application process or NULL, if an error
 *         occurred
 */
wchar_t* createCommandLine( wchar_t const * appendix )
{
    const wchar_t* CMDPREFIX = L"_";
    const wchar_t* DQUOTE = L"\"";
    const wchar_t* SPACE = L" ";

    wchar_t* cmdline = NULL;

    wchar_t cmdname[ _MAX_PATH ];
    wchar_t drive[ _MAX_DRIVE ];
    wchar_t dir[ _MAX_PATH ];
    wchar_t base[ _MAX_FNAME ];
    wchar_t newbase[ _MAX_FNAME ];
    wchar_t ext[ _MAX_EXT ];

    /* get the absolute path of the executable file */
    if ( GetModuleFileNameW( NULL, cmdname, MY_SIZE( cmdname ) ) )
    {
        /* prefix the executable file name by '_' */
        _wsplitpath( cmdname, drive, dir, base, ext );
        wcscpy( newbase, CMDPREFIX );
        wcscat( newbase, base );
        _wmakepath( cmdname, drive, dir, newbase, ext );

        /* create the command line */
        cmdline = (wchar_t*) malloc( (wcslen( DQUOTE ) + wcslen( cmdname ) +
            wcslen ( DQUOTE ) + wcslen( SPACE ) + wcslen( appendix ) + 1) * sizeof(wchar_t) );
        wcscpy( cmdline, DQUOTE );
        wcscat( cmdline, cmdname );
        wcscat( cmdline, DQUOTE );
        wcscat( cmdline, SPACE );
        wcscat( cmdline, appendix );
    }

    return cmdline;
}

/*
 * Gets the pointer to the error file.
 *
 * <p>The error file will only be created, if create != 0.</p>
 *
 * <p>The error file has the name <executable file name>-error.log and is
 * created in the same directory as the executable file. If this fails,
 * the error file is created in the directory designated for temporary files.
 * </p>

 * @param create specifies, if the error file should be created (create != 0)
 *
 * @return the pointer to the open error file or NULL, if no error file is
 *         open or can be created
 */
FILE* getErrorFile( int create )
{
    const wchar_t* MODE = L"w";
    const wchar_t* BASEPOSTFIX = L"-error";
    const wchar_t* EXTENSION = L".log";

    static FILE* ferr = NULL;

    wchar_t fname[ _MAX_PATH ];
    wchar_t drive[ _MAX_DRIVE ];
    wchar_t dir[ _MAX_PATH ];
    wchar_t base[ _MAX_FNAME ];
    wchar_t newbase[ _MAX_FNAME ];
    wchar_t ext[ _MAX_EXT ];

    if ( ferr == NULL && create )
    {
        /* get the absolute path of the executable file */
        if ( GetModuleFileNameW( NULL, fname, MY_SIZE( fname ) ) )
        {
            /* create error file in the directory of the executable file */
            _wsplitpath( fname, drive, dir, base, ext );
            wcscpy( newbase, base );
            wcscat( newbase, BASEPOSTFIX );
            _wmakepath( fname, drive, dir, newbase, EXTENSION );
            ferr = _wfopen( fname, MODE );

            if ( ferr == NULL )
            {
                /* create error file in the temp directory */
                GetTempPathW(MY_SIZE( fname ), fname );
                wcscat( fname, newbase );
                wcscat( fname, EXTENSION );
                ferr = _wfopen( fname, MODE );
            }
        }
    }

    return ferr;
}

/*
 * Writes an error message to the error file.
 *
 * @param errstr specifies the error message
 */
void writeError( const char* errstr )
{
    FILE* ferr = getErrorFile( 1 );
    if ( ferr != NULL )
    {
        fputs( errstr, ferr );
        fflush( ferr );
    }
}

/*
 * Closes the error file.
 */
void closeErrorFile()
{
    FILE* ferr = getErrorFile( 0 );
    if ( ferr != NULL )
        fclose( ferr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
