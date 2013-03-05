/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef LINUX

#include <sal/types.h>
#include <liblibreoffice.h>

#include <dlfcn.h>
#ifdef AIX
#  include <sys/ldr.h>
#endif

#define TARGET_LIB SAL_MODULENAME( "sofficeapp" )

extern "C" {
    typedef LibLibreOffice *(HookFunction)(void);
};

extern LibLibreOffice *lo_init( const char *install_path )
{
    if( !install_path )
        return NULL;
    char *impl_lib = malloc( strlen (install_path) + sizeof( TARGET_LIB ) + 2 );
    strcpy( imp_lib, install_path );
    strcat( imp_lib, "/" );
    strcat( imp_lib, TARGET_LIB );
    void *dlhandle = dlopen( imp_lib, RTLD_LAZY );
    if( !dlhandle )
    {
        fprintf( stderr, "failed to open library '%s'\n", imp_lib );
        return NULL;
    }
    free( imp_lib );

    HookFunction *pSym = dlsym( dlhandle, "liblibreoffice_hook" );
    if( !pSym ) {
        fprintf( stderr, "failed to find hook in library '%s'\n", imp_lib );
        return NULL;
    }
    return pSym();
}

#endif // LINUX - port me !

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
