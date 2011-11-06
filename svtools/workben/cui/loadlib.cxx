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

