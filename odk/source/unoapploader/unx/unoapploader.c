/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoapploader.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:19:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef LINUX
#define __USE_GNU
#endif
#include <dlfcn.h>

char* getPath();
char* getPathFromPathEnvVar();
char* createCommandName( char* argv0 );

const int SEPARATOR = '/';
const char* PATHSEPARATOR = ":";


/*
 * The main function implements a loader for applications which use UNO.
 *
 * <p>This code runs on the Unix/Linux platforms only.</p>
 *
 * <p>The main function detects a UNO installation on the system and adds the
 * program directory of the UNO installation to the LD_LIBRARY_PATH environment
 * variable. After that, the application process is loaded and started, whereby
 * the new process inherits the environment of the calling process, including
 * the modified LD_LIBRARY_PATH environment variable. The application's
 * executable name must be the same as the name of this executable, prefixed
 * by '_'.</p>
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
    const char* ENVVARNAME = "LD_LIBRARY_PATH";

    char* path;
    char* value;
    char* envstr;
    char* cmdname;
    int size;

    /* get the path of the UNO installation */
    path = getPath();

    if ( path != NULL )
    {
        /* get the value of the LD_LIBRARY_PATH environment variable */
        value = getenv( ENVVARNAME );

        /*
         * add the UNO installation path to the LD_LIBRARY_PATH environment
         * variable; note that this only affects the environment variable of the
         * current process, the command processor's environment is not changed
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
        putenv( envstr );
    }
    else
    {
        fprintf( stderr, "Warning: no UNO installation found!\n" );
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
 *         found, or if an error occured
 */
char* getPath()
{
    const char* UNOPATHVARNAME = "UNO_PATH";

    char* path = NULL;

    /* get the installation path from the UNO_PATH environment variable */
    path = getenv( UNOPATHVARNAME );

    if ( path == NULL || strlen( path ) == 0 )
    {
        /* get the installation path from the PATH environment variable */
        path = getPathFromPathEnvVar();

        if ( path == NULL )
        {
            fprintf( stderr, "Warning: getting path from PATH environment "
                "variable failed!\n" );
            fflush( stderr );
        }
    }

    return path;
}

/*
 * Gets the installation path from the PATH environment variable.
 *
 * <p>An installation is found, if the executable 'soffice' or a symbolic link
 * is in one of the directories listed in the PATH environment variable.</p>
 *
 * @return the installation path or NULL, if no installation was found or
 *         if an error occured
 */
char* getPathFromPathEnvVar()
{
    const char* PATHVARNAME = "PATH";
    const char* APPENDIX = "/soffice";

    char* path = NULL;
    char* env = NULL;
    char* str = NULL;
    char* dir = NULL;
    char* file = NULL;
    char* resolved = NULL;
    char* sep = NULL;

    char buffer[1024];
    int pos;

    /* get the value of the PATH environment variable */
    env = getenv( PATHVARNAME );
    str = (char*) malloc( strlen( env ) + 1 );
    strcpy( str, env );

    /* get the tokens separated by ':' */
    dir = strtok( str, PATHSEPARATOR );

    while ( dir )
    {
        /* construct soffice file path */
        file = (char*) malloc( strlen( dir ) + strlen( APPENDIX ) + 1 );
        strcpy( file, dir );
        strcat( file, APPENDIX );

        /* check existence of soffice file */
        if ( !access( file, F_OK ) )
        {
            /* resolve symbolic link */
            resolved = realpath( file, buffer );

            if ( resolved != NULL )
            {
                /* get path to program directory */
                sep = strrchr( resolved, SEPARATOR );

                if ( sep != NULL )
                {
                    pos = sep - resolved;
                    path = (char*) malloc( pos + 1 );
                    strncpy( path, resolved, pos );
                    path[ pos ] = '\0';
                    free( file );
                    break;
                }
            }
        }

        dir = strtok( NULL, PATHSEPARATOR );
        free( file );
    }

    free( str );

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
 * @return the application's executable file name or NULL, if an error occured
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
