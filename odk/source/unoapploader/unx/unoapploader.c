/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

char const* getPath();
char* createCommandName( char* argv0 );

const int SEPARATOR = '/';
const char* PATHSEPARATOR = ":";


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
    char const* path;
    char* cmdname;

    (void) argc; /* avoid warning about unused parameter */

    /* get the path of the UNO installation */
    path = getPath();

    if ( path != NULL )
    {
#ifdef MACOSX
        static const char* ENVVARNAME = "DYLD_LIBRARY_PATH";
#else
        static const char* ENVVARNAME = "LD_LIBRARY_PATH";
#endif
        char * libpath;
        int freeLibpath;

        char* value;
        char* envstr;
        int size;

        size_t pathlen = strlen(path);
        struct stat stat;
        int ret;
        char * unoinfo = malloc(
            pathlen + RTL_CONSTASCII_LENGTH("/unoinfo") + 1);
            /*TODO: overflow */
        if (unoinfo == NULL) {
            fprintf(stderr, "Error: out of memory!\n");
            exit(EXIT_FAILURE);
        }
        strcpy(unoinfo, path);
        strcpy(
            unoinfo + pathlen,
            "/unoinfo" + (pathlen == 0 || path[pathlen - 1] != '/' ? 0 : 1));
        ret = lstat(unoinfo, &stat);
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
            freeLibpath = 1;
        }
        else
        {
            /* Assume an old OOo 2.x installation without unoinfo: */
            libpath = (char *) path;
            freeLibpath = 0;
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
        if ( freeLibpath != 0 )
        {
            free( libpath );
        }
        if ( value != NULL )
        {
            strcat( envstr, PATHSEPARATOR );
            strcat( envstr, value );
        }
        putenv( envstr );
#ifdef MACOSX
        /* https://bz.apache.org/ooo/show_bug.cgi?id=127965 */
        value = getenv( "PATH" );
        size = strlen( "PATH" ) + strlen( "=/usr/local/bin" ) + 1;
        if ( value != NULL )
            size += strlen( PATHSEPARATOR ) + strlen( value );
        envstr = (char*) malloc( size );
        strcpy( envstr, "PATH=" );
        if ( value != NULL ) {
            strcat( envstr, value);
            strcat( envstr, PATHSEPARATOR);
        }
        strcat( envstr, "/usr/local/bin" ); /* We are adding at the end */
        putenv( envstr );
#endif
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
    execvp( cmdname, argv );
    fprintf( stderr, "Error: execvp failed!\n" );
    fflush( stderr );

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
    {
        fprintf( stderr, "Warning: getting path from PATH environment variable failed!\n" );
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
    Dl_info dl_info;
    int pos;

    /* get the executable file name from argv0 */
    prgname = argv0;

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

    /* prefix the executable file name by '_' */
    if ( prgname != NULL )
    {
        cmdname = (char*) malloc( strlen( prgname ) + strlen( CMDPREFIX ) + 1 );
        sep = strrchr( prgname, SEPARATOR );
        if ( sep != NULL )
        {
            pos = ++sep - prgname;
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
