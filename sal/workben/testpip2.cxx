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



