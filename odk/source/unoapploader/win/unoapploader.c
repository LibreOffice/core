/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoapploader.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:20:29 $
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
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <Windows.h>

char* getPath();
char* getPathFromWindowsRegistry();
char* getPathFromRegistryKey( HKEY hroot, const char* subKeyName );
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
 * default value of the key "Software\OpenOffice.org\UNO\InstallPath" from the
 * root key HKEY_CURRENT_USER in the Windows Registry. If this key is missing,
 * the key is read from the root key HKEY_LOCAL_MACHINE.</p>
 */
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow )
{
    const char* ENVVARNAME = "PATH";
    const char* PATHSEPARATOR = ";";

    char* path = NULL;
    char* value = NULL;
    char* envstr = NULL;
    char* cmdline = NULL;
    int size;
    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_info;
    BOOL bCreate;

    /* get the path of the UNO installation */
    path = getPath();

    if ( path != NULL )
    {
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
        /* get the installation path from the Windows Registry */
        path = getPathFromWindowsRegistry();
        if ( path == NULL )
            writeError( "Warning: getting path from Windows Registry "
                        "failed!\n" );
    }

    return path;
}


/*
 * Gets the installation path from the Windows Registry.
 *
 * @return the installation path or NULL, if no installation was found or
 *         if an error occured
 */
char* getPathFromWindowsRegistry()
{
    const char* SUBKEYNAME = "Software\\OpenOffice.org\\UNO\\InstallPath";

    char* path = NULL;

    /* read the key's default value from HKEY_CURRENT_USER */
    path = getPathFromRegistryKey( HKEY_CURRENT_USER, SUBKEYNAME );

    if ( path == NULL )
    {
        /* read the key's default value from HKEY_LOCAL_MACHINE */
        path = getPathFromRegistryKey( HKEY_LOCAL_MACHINE, SUBKEYNAME );
    }

    return path;
}

/*
 * Gets the installation path from the Windows Registry for the specified
 * registry key.
 *
 * @param hroot       open handle to predefined root registry key
 * @param subKeyName  name of the subkey to open
 *
 * @return the installation path or NULL, if no installation was found or
 *         if an error occured
 */
char* getPathFromRegistryKey( HKEY hroot, const char* subKeyName )
{
    HKEY hkey;
    DWORD type;
    char* data = NULL;
    DWORD size;

    /* open the specified registry key */
    if ( RegOpenKeyEx( hroot, subKeyName, 0, KEY_READ, &hkey ) != ERROR_SUCCESS )
    {
        return NULL;
    }

    /* find the type and size of the default value */
    if ( RegQueryValueEx( hkey, NULL, NULL, &type, NULL, &size) != ERROR_SUCCESS )
    {
        RegCloseKey( hkey );
        return NULL;
    }

    /* get memory to hold the default value */
    data = (char*) malloc( size );

    /* read the default value */
    if ( RegQueryValueEx( hkey, NULL, NULL, &type, data, &size ) != ERROR_SUCCESS )
    {
        RegCloseKey( hkey );
        return NULL;
    }

    /* release registry key handle */
    RegCloseKey( hkey );

    return data;
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
 *         occured
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
        fprintf( ferr, errstr );
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
