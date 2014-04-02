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

#include "sal/config.h"

#include <stdlib.h>
#include <string.h>

#include <cppuhelper/findsofficepath.h>

#if defined WNT

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
 * Gets the installation path from the Windows Registry for the specified
 * registry key.
 *
 * @param hroot       open handle to predefined root registry key
 * @param subKeyName  name of the subkey to open
 *
 * @return the installation path or NULL, if no installation was found or
 *         if an error occurred
 */
static char* getPathFromRegistryKey( HKEY hroot, const char* subKeyName )
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
    if ( RegQueryValueEx( hkey, NULL, NULL, &type, (LPBYTE) data, &size ) != ERROR_SUCCESS )
    {
        RegCloseKey( hkey );
        return NULL;
    }

    /* release registry key handle */
    RegCloseKey( hkey );

    return data;
}

/*
 * Gets the installation path from the Windows Registry.
 *
 * @return the installation path or NULL, if no installation was found or
 *         if an error occurred
 */
static char* platformSpecific()
{
    const char* SUBKEYNAME = "Software\\LibreOffice\\UNO\\InstallPath";

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

#else

#include <unistd.h>
#include <limits.h>

/*
 * Gets the installation path from the PATH environment variable.
 *
 * <p>An installation is found, if the executable 'soffice' or a symbolic link
 * is in one of the directories listed in the PATH environment variable.</p>
 *
 * @return the installation path or NULL, if no installation was found or
 *         if an error occurred
 */
static char* platformSpecific()
{
    const int SEPARATOR = '/';
    const char* PATHSEPARATOR = ":";
    const char* PATHVARNAME = "PATH";
    const char* APPENDIX = "/libreoffice";

    char* path = NULL;
    char* env = NULL;
    char* str = NULL;
    char* dir = NULL;
    char* file = NULL;
    char* resolved = NULL;
    char* sep = NULL;

    char buffer[PATH_MAX];
    int pos;

    /* get the value of the PATH environment variable */
    env = getenv( PATHVARNAME );
    if (env == NULL)
        return NULL;
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

#endif

char const* cppuhelper_detail_findSofficePath()
{
    const char* UNOPATHVARNAME = "UNO_PATH";

    char* path = NULL;

    /* get the installation path from the UNO_PATH environment variable */
    path = getenv( UNOPATHVARNAME );

    if (!path || !path[0])
        path = platformSpecific();

    return path;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
