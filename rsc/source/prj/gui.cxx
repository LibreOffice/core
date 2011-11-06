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
#if defined( UNX ) || ( defined( OS2 ) && ( defined( TCPP ) || defined ( GCC )) ) ||  defined (WTC) || defined (MTW) || defined(__MINGW32__)
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
#ifdef MTW
    RscCmdLine* pCmdLine   = new RscCmdLine( argc, (char **)argv, pErrHdl );
#else
    RscCmdLine* pCmdLine   = new RscCmdLine( argc, argv, pErrHdl );
#endif
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
