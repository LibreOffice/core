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
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include "cppuhelper/findsofficepath.h"
#include "sal/types.h"

#define MY_LENGTH(s) (sizeof (s) / sizeof *(s) - 1)

char const* getPath();
char* createCommandLine( char* lpCmdLine );
FILE* getErrorFile( int create );
void writeError( const char* errstr );
void closeErrorFile();

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
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow )
{
    const char* ENVVARNAME = "PATH";
    const char* PATHSEPARATOR = ";";

    char const* path = NULL;
    char path2[MAX_PATH];
    char* value = NULL;
    char* envstr = NULL;
    char* cmdline = NULL;
    size_t size;
    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_info;
    BOOL bCreate;

    (void) hInstance; /* unused */
    (void) hPrevInstance; /* unused */
    (void) nCmdShow; /* unused */

    /* get the path of the UNO installation */
    path = getPath();

    if ( path != NULL )
    {
        wchar_t cmd[
            MY_LENGTH(L"\"") + MAX_PATH +
            MY_LENGTH(L"\\unoinfo.exe\" c++")];
            /* hopefully does not overflow */
        int pathsize;
        SECURITY_ATTRIBUTES sec;
        HANDLE temp;
        HANDLE stdoutRead;
        HANDLE stdoutWrite;
        STARTUPINFOW startinfo;
        PROCESS_INFORMATION procinfo;
        int ret;
        cmd[0] = L'"';
        pathsize = MultiByteToWideChar(CP_ACP, 0, path, -1, cmd + 1, MAX_PATH);
        if (pathsize == 0) {
            writeError("Error: MultiByteToWideChar failed!\n");
            closeErrorFile();
            return 1;
        }
        if (wcschr(cmd + 1, L'"') != NULL) {
            writeError("Error: bad characters in UNO installation path!\n");
            closeErrorFile();
            return 1;
        }
        wcscpy(
            cmd + pathsize,
            &L"\\unoinfo.exe\" c++"[
                pathsize == 1 || cmd[pathsize - 1] != L'\\' ? 0 : 1]);
        sec.nLength = sizeof (SECURITY_ATTRIBUTES);
        sec.lpSecurityDescriptor = NULL;
        sec.bInheritHandle = TRUE;
        if (CreatePipe(&temp, &stdoutWrite, &sec, 0) == 0 ||
            DuplicateHandle(
                GetCurrentProcess(), temp, GetCurrentProcess(), &stdoutRead, 0,
                FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS) == 0)
        {
            writeError("Error: CreatePipe/DuplicateHandle failed!\n");
            closeErrorFile();
            return 1;
        }
        memset(&startinfo, 0, sizeof (STARTUPINFOW));
        startinfo.cb = sizeof (STARTUPINFOW);
        startinfo.lpDesktop = L"";
        startinfo.dwFlags = STARTF_USESTDHANDLES;
        startinfo.hStdOutput = stdoutWrite;
        ret = CreateProcessW(
            NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &startinfo, &procinfo);
        if (ret != 0) {
            char * buf = NULL;
            char * tmp;
            DWORD n = 1000;
            DWORD k = 0;
            DWORD exitcode;
            int path2size;
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
            if (k == 0) {
                path2size = 0;
            } else {
                path2size = WideCharToMultiByte(
                    CP_ACP, 0, (wchar_t *) buf, k / 2, path2, MAX_PATH - 1,
                    NULL, NULL);
                if (path2size == 0) {
                    writeError("Error: converting unoinfo output failed!\n");
                    closeErrorFile();
                    return 1;
                }
            }
            path2[path2size] = '\0';
            path = path2;
        } else {
            if (GetLastError() != ERROR_FILE_NOT_FOUND) {
                writeError("Error: calling unoinfo failed!\n");
                closeErrorFile();
                return 1;
            }
            CloseHandle(stdoutRead);
            CloseHandle(stdoutWrite);
        }

        /* get the value of the PATH environment variable */
        value = getenv( ENVVARNAME );

        /*
         * add the UNO installation path to the PATH environment variable;
         * note that this only affects the environment variable of the current
         * process, the command processor's environment is not changed
         */
        size = strlen( ENVVARNAME ) + strlen( "=" ) + strlen( path ) + 1;
        if ( value != NULL )
            size += strlen( PATHSEPARATOR ) + strlen( value );
        envstr = (char*) malloc( size );
        strcpy( envstr, ENVVARNAME );
        strcat( envstr, "=" );
        strcat( envstr, path );
        if ( value != NULL )
        {
            strcat( envstr, PATHSEPARATOR );
            strcat( envstr, value );
        }
        _putenv( envstr );
        free( envstr );
    }
    else
    {
        writeError( "Warning: no UNO installation found!\n" );
    }

    /* create the command line for the application process */
    cmdline = createCommandLine( lpCmdLine );
    if ( cmdline == NULL )
    {
        writeError( "Error: cannot create command line!\n" );
        closeErrorFile();
        return 1;
    }

    /* create the application process */
    memset( &startup_info, 0, sizeof( STARTUPINFO ) );
    startup_info.cb = sizeof( STARTUPINFO );
    bCreate = CreateProcess( NULL, cmdline, NULL,  NULL, FALSE, 0, NULL, NULL,
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
 *         found, or if an error occurred
 */
char const* getPath()
{
    char const* path = cppuhelper_detail_findSofficePath();

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
char* createCommandLine( char* appendix )
{
    const char* CMDPREFIX = "_";
    const char* DQUOTE = "\"";
    const char* SPACE = " ";

    char* cmdline = NULL;

    char cmdname[ _MAX_PATH ];
    char drive[ _MAX_DRIVE ];
    char dir[ _MAX_PATH ];
    char base[ _MAX_FNAME ];
    char newbase[ _MAX_FNAME ];
    char ext[ _MAX_EXT ];

    /* get the absolute path of the executable file */
    if ( GetModuleFileName( NULL, cmdname, sizeof( cmdname ) ) )
    {
        /* prefix the executable file name by '_' */
        _splitpath( cmdname, drive, dir, base, ext );
        strcpy( newbase, CMDPREFIX );
        strcat( newbase, base );
        _makepath( cmdname, drive, dir, newbase, ext );

        /* create the command line */
        cmdline = (char*) malloc( strlen( DQUOTE ) + strlen( cmdname ) +
            strlen ( DQUOTE ) + strlen( SPACE ) + strlen( appendix ) + 1 );
        strcpy( cmdline, DQUOTE );
        strcat( cmdline, cmdname );
        strcat( cmdline, DQUOTE );
        strcat( cmdline, SPACE );
        strcat( cmdline, appendix );
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
    const char* MODE = "w";
    const char* BASEPOSTFIX = "-error";
    const char* EXTENSION = ".log";

    static FILE* ferr = NULL;

    char fname[ _MAX_PATH ];
    char drive[ _MAX_DRIVE ];
    char dir[ _MAX_PATH ];
    char base[ _MAX_FNAME ];
    char newbase[ _MAX_FNAME ];
    char ext[ _MAX_EXT ];

    if ( ferr == NULL && create )
    {
        /* get the absolute path of the executable file */
        if ( GetModuleFileName( NULL, fname, sizeof( fname ) ) )
        {
            /* create error file in the directory of the executable file */
            _splitpath( fname, drive, dir, base, ext );
            strcpy( newbase, base );
            strcat( newbase, BASEPOSTFIX );
            _makepath( fname, drive, dir, newbase, EXTENSION );
            ferr = fopen( fname, MODE );

            if ( ferr == NULL )
            {
                /* create error file in the temp directory */
                GetTempPath( sizeof( fname ), fname );
                strcat( fname, newbase );
                strcat( fname, EXTENSION );
                ferr = fopen( fname, MODE );
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
