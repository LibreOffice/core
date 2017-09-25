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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef LINUX
#define __USE_GNU
#endif
#include <dlfcn.h>

#include "cppuhelper/findsofficepath.h"
#include "rtl/string.h"
#include "sal/types.h"

char* getPath(void);
char* createCommandName( char* argv0 );

static const int SEPARATOR = '/';
static const char* PATHSEPARATOR = ":";


/*
 * The main function implements a loader for applications which use UNO.
 *
 * <p>This code runs on the Unix/Linux platforms only.</p>
 *
 * <p>The main function detects a UNO installation on the system and adds the
 * relevant directories of the installation to the LD_LIBRARY_PATH environment
 * variable. After that, the application process is loaded and started, whereby
 * the new process inherits the environment of the calling process, including
 * the modified LD_LIBRARY_PATH environment variable. The application's
 * executable name must be the same as the name of this executable, prefixed
 * by '_'.</p>
 * <p>On MACOSX DYLD_LIBRARY_PATH is used instead of LD_LIBRARY_PATH!<p>
 *
 * <p>A UNO installation can be specified by the user by setting the UNO_PATH
 * environment variable to the program directory of the UNO installation.
 * If no installation is specified by the user, the default installation on
 * the system will be taken. The default installation is found from the
 * PATH environment variable. This requires that the 'soffice' executable or
 * a symbolic link is in one of the directories listed in the PATH environment
 * variable.</p>
 */
int main( int argc, char *argv[] )
{
    char* path;
    char* cmdname;

    (void) argc; /* avoid warning about unused parameter */

    /* get the path of the UNO installation */
    path = getPath();

    if ( path != NULL )
    {
#if defined(MACOSX)
        static const char* ENVVARNAME = "DYLD_LIBRARY_PATH";
#elif defined(AIX)
        static const char* ENVVARNAME = "LIBPATH";
#else
        static const char* ENVVARNAME = "LD_LIBRARY_PATH";
#endif
        char* libpath;
        char* value;
        char* envstr;
        int size;

        size_t pathlen = strlen(path);
        struct stat stats;
        int ret;

        static char const unoinfoSuffix[] = "/unoinfo";
        char * unoinfo = malloc(
            pathlen + RTL_CONSTASCII_LENGTH(unoinfoSuffix) + 1);
            /*TODO: overflow */
        if (unoinfo == NULL) {
            free(path);
            fprintf(stderr, "Error: out of memory!\n");
            exit(EXIT_FAILURE);
        }
        strcpy(unoinfo, path);
        strcpy(
            unoinfo + pathlen,
            unoinfoSuffix + (pathlen == 0 || path[pathlen - 1] != '/' ? 0 : 1));
        ret = lstat(unoinfo, &stats);
        free(unoinfo);

        if (ret == 0) {
            char * cmd = malloc(
                2 * pathlen + RTL_CONSTASCII_LENGTH("/unoinfo c++") + 1);
                /*TODO: overflow */
            char const * p;
            char * q;
            FILE * f;
            size_t n = 1000;
            size_t old = 0;
            if (cmd == NULL) {
                fprintf(stderr, "Error: out of memory!\n");
                exit(EXIT_FAILURE);
            }
            p = path;
            q = cmd;
            while (*p != '\0') {
                *q++ = '\\';
                *q++ = *p++;
            }
            if (p == path || p[-1] != '/') {
                *q++ = '/';
            }
            strcpy(q, "unoinfo c++");
            f = popen(cmd, "r");
            free(cmd);
            if (f == NULL)
            {
                fprintf(stderr, "Error: calling unoinfo failed!\n");
                exit(EXIT_FAILURE);
            }
            libpath = NULL;
            for (;;) {
                size_t m;
                libpath = realloc(libpath, n);
                if (libpath == NULL) {
                    fprintf(
                        stderr,
                        "Error: out of memory reading unoinfo output!\n");
                    exit(EXIT_FAILURE);
                }
                m = fread(libpath + old, 1, n - old - 1, f);
                if (m != n - old - 1) {
                    if (ferror(f)) {
                        fprintf(stderr, "Error: cannot read unoinfo output!\n");
                        exit(EXIT_FAILURE);
                    }
                    libpath[old + m] = '\0';
                    break;
                }
                if (n >= SAL_MAX_SIZE / 2) {
                    fprintf(
                        stderr,
                        "Error: out of memory reading unoinfo output!\n");
                    exit(EXIT_FAILURE);
                }
                old = n - 1;
                n *= 2;
            }
            if (pclose(f) != 0) {
                fprintf(stderr, "Error: executing unoinfo failed!\n");
                exit(EXIT_FAILURE);
            }
            free(path);
        }
        else
        {
            /* Assume an old OOo 2.x installation without unoinfo: */
            libpath = path;
        }

        value = getenv( ENVVARNAME );

        // workaround for finding wrong libsqlite3.dylib in the office installation
        // For MacOS > 10.6 nss uses the system lib -> unresolved symbol _sqlite3_wal_checkpoint
#ifdef MACOSX
        size = strlen( ENVVARNAME ) + strlen( "=/usr/lib:" ) + strlen( libpath ) + 1;
#else
        size = strlen( ENVVARNAME ) + strlen( "=" ) + strlen( libpath ) + 1;
#endif
        if ( value != NULL )
            size += strlen( PATHSEPARATOR ) + strlen( value );
        envstr = (char*) malloc( size );
        strcpy( envstr, ENVVARNAME );
#ifdef MACOSX
        strcat( envstr, "=/usr/lib:" );
#else
        strcat( envstr, "=" );
#endif
        strcat( envstr, libpath );
        free( libpath );
        if ( value != NULL )
        {
            strcat( envstr, PATHSEPARATOR );
            strcat( envstr, value );
        }
        putenv( envstr );
    }
    else
    {
        fprintf( stderr, "Warning: no office installation found!\n" );
        fflush( stderr );
    }

    /* set the executable name for the application process */
    cmdname = createCommandName( argv[0] );
    argv[0] = cmdname;

    /*
     * create the application process;
     * if successful, execvp doesn't return to the calling process
     */
    /* coverity[tainted_string] - createCommandName creates a safe string */
    execvp( cmdname, argv );
    fprintf( stderr, "Error: execvp failed!\n" );
    fflush( stderr );

    return 0;
}

/*
 * Gets the path of a UNO installation.
 *
 * @return the installation path or NULL, if no installation was specified or
 *         found, or if an error occurred.
 *         Returned pointer must be released with free()
 */
char* getPath(void)
{
    char* path = cppuhelper_detail_findSofficePath();

    if ( path == NULL )
    {
        fprintf( stderr, "Warning: getting path from PATH environment "
                 "variable failed!\n" );
        fflush( stderr );
    }

    return path;
}

/*
 * Creates the application's executable file name.
 *
 * <p>The application's executable file name is the name of this executable
 * prefixed by '_'.</p>
 *
 * @param argv0 specifies the argv[0] parameter of the main function
 *
 * @return the application's executable file name or NULL, if an error occurred
 */
char* createCommandName( char* argv0 )
{
    const char* CMDPREFIX = "_";
    const char* prgname = NULL;

    char* cmdname = NULL;
    char* sep = NULL;
#ifndef AIX
    Dl_info dl_info;
#endif

    /* get the executable file name from argv0 */
    prgname = argv0;

#ifndef AIX
    /*
     * if argv0 doesn't contain an absolute path name, try to get the absolute
     * path name from dladdr; note that this only works for Solaris, not for
     * Linux
     */
    if ( argv0 != NULL && *argv0 != SEPARATOR &&
         dladdr( (void*) &createCommandName, &dl_info ) &&
         dl_info.dli_fname != NULL && *dl_info.dli_fname == SEPARATOR )
    {
        prgname = dl_info.dli_fname;
    }
#endif

    /* prefix the executable file name by '_' */
    if ( prgname != NULL )
    {
        cmdname = (char*) malloc( strlen( prgname ) + strlen( CMDPREFIX ) + 1 );
        sep = strrchr( prgname, SEPARATOR );
        if ( sep != NULL )
        {
            int pos = ++sep - prgname;
            strncpy( cmdname, prgname, pos );
            cmdname[ pos ] = '\0';
            strcat( cmdname, CMDPREFIX );
            strcat( cmdname, sep );
        }
        else
        {
            strcpy( cmdname, CMDPREFIX );
            strcat( cmdname, prgname );
        }
    }

    return cmdname;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
