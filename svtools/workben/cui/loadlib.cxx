/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loadlib.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:33:27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <tools/string.hxx>
#include <osl/module.h>
#include <rtl/ustring.hxx>

using namespace rtl;

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
        void* pFunc = osl_getSymbol( aLibrary, OUString::createFromAscii( "GetVersionInfo" ).pData );
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

