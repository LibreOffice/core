/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef LINUX

#include <stdio.h>
#include <string.h>

#include <osl/module.h>
#include <sal/types.h>
#include <LibreOfficeKit/LibreOfficeKit.h>

#include <dlfcn.h>
#ifdef AIX
#  include <sys/ldr.h>
#endif

#define TARGET_LIB SAL_MODULENAME( "sofficeapp" )

typedef LibreOfficeKit *(HookFunction)(void);

SAL_DLLPUBLIC_EXPORT LibreOfficeKit *lok_init( const char *install_path )
{
    char *imp_lib;
    void *dlhandle;
    HookFunction *pSym;

    if( !install_path )
        return NULL;
    if( !( imp_lib = (char *) malloc( strlen (install_path) + sizeof( TARGET_LIB ) + 2 ) ) )
    {
        fprintf( stderr, "failed to open library : not enough memory\n");
        return NULL;
    }

    strcpy( imp_lib, install_path );
    strcat( imp_lib, "/" );
    strcat( imp_lib, TARGET_LIB );

    if( !( dlhandle = dlopen( imp_lib, RTLD_LAZY ) ) )
    {
        fprintf( stderr, "failed to open library '%s'\n", imp_lib );
        free( imp_lib );
        return NULL;
    }

    pSym = (HookFunction *) dlsym( dlhandle, "libreofficekit_hook" );
    if( !pSym ) {
        fprintf( stderr, "failed to find hook in library '%s'\n", imp_lib );
        dlclose( dlhandle );
        free( imp_lib );
        return NULL;
    }

    free( imp_lib );
    return pSym();
}

#endif // not LINUX => port me !

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
