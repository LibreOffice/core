/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gui.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 13:08:05 $
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
#include "precompiled_rsc.hxx"

#include <stdlib.h>
#include <stdio.h>

#ifndef _RSCRSC_HXX
#include <rscrsc.hxx>
#endif
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif

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
