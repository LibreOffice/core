/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testpip2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:10:27 $
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
#include "precompiled_sal.hxx"

#include <stdio.h>
#include <stdlib.h>

#define INCL_DOSPROCESS
#include <osl/pipe.h>
#include <os2.h>

// eindeutiger Name fÅr die Pipe
const char pszPipeName[] = "TestPipe";

oslPipe Pipe;


void fail( const char * pszText, int retval )
{
    fprintf( stderr, "TestPipe Client: %s", pszText );
    fprintf( stderr, "TestPipe Client: test failed, ErrNo: %d.\n", retval );
    exit( retval );
}



/*
 * Teste die Pipe-Implementation in osl
 */

int main (void)
{
    char    szBuffer[ 256 ];
    sSize_t nChars;

    // gib dem Server die Chance, die Pipe zu îffnen
    DosSleep( 1000 );

    // erzeuge die Pipe
    Pipe = osl_createPipe( pszPipeName, osl_Pipe_OPEN, 0 );

    if( !Pipe )
        fail( "unable to open pipe.\n",
              osl_getLastPipeError(NULL));


    // empfange Daten vom Server
    nChars = osl_receivePipe( Pipe, szBuffer, 256 );

    if( nChars < 0 )
        fail( "unable to read from pipe.\n",
              osl_getLastPipeError( Pipe ) );

    printf( "TestPipe Client: data received: %s.\n", szBuffer );

    // Sende die Daten wieder zurÅck.
    nChars = osl_sendPipe( Pipe, szBuffer, nChars );

    if( nChars < 0 )
        fail( "unable to write on pipe.\n",
              osl_getLastPipeError( Pipe ) );

    // schliesse die Pipe
    osl_destroyPipe( Pipe );

    printf( "TestPipe Client: test passed.\n" );
    return 0;
}



