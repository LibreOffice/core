/*************************************************************************
 *
 *  $RCSfile: gui.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#ifdef MAC
#include <cursorctl.h>
#endif

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
#if defined( UNX ) || ( defined( PM2 ) && ( defined( TCPP ) || defined ( GCC )) ) || defined( MAC ) || defined (WTC) || defined (MTW)
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

void PrimeNumber(){
    USHORT i, n;
    for( i = 801; i < 1000; i += 2 ){
        for( n = 2; n < i && ((i % n) != 0); n++ );
        if( n == i ){
            printf( "\nPrimzahl: %d\n", i );
            return;
        }
    }
}


#if defined( UNX ) || defined( MAC ) || ( defined( PM2 ) && ( defined( CSET ) || defined ( GCC ))) || defined (WTC) || defined(ICC)
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
#ifdef MAC
    InitCursorCtl( 0 );
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
                                             pCmdLine->nLangTypeId,
                                             pCmdLine->nByteOrder,
                                             pCmdLine->nSourceCharSet,
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

void RscExit( USHORT nExit )
{
    if( nExit )
        printf( "Program exit is %d\n", nExit );
    exit( nExit );
}
