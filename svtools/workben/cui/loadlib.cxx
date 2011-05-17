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
#include "precompiled_svtools.hxx"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <tools/string.hxx>
#include <osl/module.h>
#include <rtl/ustring.hxx>

using ::rtl::OUString;

extern "C" {
struct VersionInfo
{
        const char*     pTime;
        const char*     pDate;
        const char*     pUpd;
        const char*     pMinor;
        const char*     pBuild;
        const char*     pInpath;
};

typedef VersionInfo*(__LOADONCALLAPI *GetVersionInfo)(void);
}

int __LOADONCALLAPI main( int argc, char **argv )
{
    VersionInfo *pInfo = NULL;

    if ( argc != 2 )
    {
        fprintf( stderr, "USAGE: %s DllName \n", argv[0] );
        exit(0);
    }
    OUString aLib = OUString::createFromAscii(argv[1]);
    oslModule aLibrary = osl_loadModule( aLib.pData, SAL_LOADMODULE_DEFAULT );
    if ( aLibrary )
    {
        void* pFunc = osl_getSymbol( aLibrary, OUString( RTL_CONSTASCII_USTRINGPARAM( "GetVersionInfo" )).pData );
        if ( pFunc )
            pInfo = (*(GetVersionInfo)pFunc)();
    }
    if ( pInfo )
    {
        fprintf( stdout, "Date : %s\n", pInfo->pDate );
        fprintf( stdout, "Time : %s\n", pInfo->pTime );
        fprintf( stdout, "UPD : %s\n", pInfo->pUpd );
        fprintf( stdout, "Minor : %s\n", pInfo->pMinor );
        fprintf( stdout, "Build : %s\n", pInfo->pBuild );
        fprintf( stdout, "Inpath : %s\n", pInfo->pInpath );
    }
    else
        fprintf( stderr, "VersionInfo not Found !\n" );

    if ( aLibrary )
        osl_unloadModule( aLibrary );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
