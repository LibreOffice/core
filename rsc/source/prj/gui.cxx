/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gui.cxx,v $
 * $Revision: 1.11 $
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

#if defined( UNX ) || ( defined( OS2 ) && ( defined( CSET ) || defined ( GCC ))) || defined (WTC) || defined(ICC) || defined(__MINGW32__)
int main ( int argc, char ** argv) {
#else
#if defined( MTW )
int  main ( int argc, char const ** argv) {
#else
int cdecl main ( int argc, char ** argv) {
#endif
#endif
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
    RscError*   pErrHdl    = new RscError();
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
