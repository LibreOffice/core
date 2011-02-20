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
#include "precompiled_rsc.hxx"

#include <stdlib.h>
#include <stdio.h>
#include <sal/main.h>
#include <rscrsc.hxx>
#include <rscdb.hxx>

/*************** G l o b a l e   V a r i a b l e n **********************/
static RscCompiler * pRscCompiler = NULL;
/****************************************************************/
/*                                                              */
/*  Function    :   ExitProgram()                               */
/*                                                              */
/*  Description :   Gibt die Temporaeren Dateien frei.          */
/****************************************************************/
#if defined( UNX ) || defined ( GCC ) || defined(__MINGW32__)
        void ExitProgram( void ){
#else
#if defined( CSET )
    void _Optlink ExitProgram( void ){
#else
    void cdecl ExitProgram( void ){
#endif
#endif
    if( pRscCompiler )
        delete pRscCompiler;
}

RscVerbosity lcl_determineVerbosity( int argc, char ** argv )
{
    for ( int i = 0; i < argc; ++i )
    {
        if ( argv[i] == NULL )
            continue;
        if ( rsc_stricmp( argv[i], "-verbose" ) == 0 )
            return RscVerbosityVerbose;
        if ( rsc_stricmp( argv[i], "-quiet" ) == 0 )
            return RscVerbositySilent;
    }
    return RscVerbosityNormal;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
#ifndef UNX
#ifdef CSET
    atexit( ExitProgram );
#else
    atexit( ExitProgram );
#endif
#endif
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "debugging %s\n", argv[0] );
#endif

    ERRTYPE     aError;

    InitRscCompiler();
    RscError*   pErrHdl    = new RscError( lcl_determineVerbosity( argc, argv ) );
    RscCmdLine* pCmdLine   = new RscCmdLine( argc, argv, pErrHdl );
    RscTypCont* pTypCont   = new RscTypCont( pErrHdl,
                                             pCmdLine->nByteOrder,
                                             pCmdLine->aPath,
                                             pCmdLine->nCommands );

    if( pErrHdl->nErrors )
        aError = ERR_ERROR;
    else{
        RscCompiler* pCompiler = new RscCompiler( pCmdLine, pTypCont );

        pRscCompiler = pCompiler;
        aError = pCompiler->Start();
        pRscCompiler = NULL;

        delete pCompiler;
    }

    delete pTypCont;
    delete pCmdLine;
    delete pErrHdl;
    delete pHS; // wird durch InitRscCompiler erzeugt

    if( aError.IsOk() )
        return( 0 );
    else
        return( 1 );
}

void RscExit( sal_uInt32 nExit )
{
    if( nExit )
        printf( "Program exit is %ud\n", (unsigned int)nExit );
    exit( nExit );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
