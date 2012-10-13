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


#include <stdlib.h>
#include <stdio.h>
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
    void cdecl ExitProgram( void ){
#endif
    if( pRscCompiler )
        delete pRscCompiler;
}

static RscVerbosity lcl_determineVerbosity( int argc, char ** argv )
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

int rsc2_main( int argc, char **argv )
{
#ifndef UNX
    atexit( ExitProgram );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
