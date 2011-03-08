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
#include "precompiled_sal.hxx"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <osl/pipe.h>
#include <osl/process.h>
#include <rtl/ustring.h>

// eindeutiger Name fr die Pipe
const char pszPipeName[] = "TestPipe";
const char szTestString[] = "This is a test";
char       szBuffer[256];

const char *  cp;
Size_t  n;
sSize_t nChars;

// osl specific variables
oslPipe          Pipe;
oslPipe          C1Pipe;
oslProcess       Process = NULL;
oslProcessError  ProcessError;
oslProcessInfo   ProcessInfo;

void fail( const char * pszText, int retval )
{
    fprintf( stderr, "TestPipe Server: %s", pszText );
    fprintf( stderr, "TestPipe Server: test failed, ErrNo: %d.\n", retval );

    if( Process ) osl_freeProcessHandle( Process );
    exit( retval );
}



/*
 * Teste die Pipe-Implementation in osl
 */

int main (int argc, const char *argv[])
{
    // erzeuge die Pipe
    rtl_uString* ustrPipeName=0;
    rtl_uString* ustrExeName=0;


    rtl_uString_newFromAscii(&ustrPipeName,pszPipeName);
    rtl_uString_newFromAscii(&ustrExeName, "//./tmp/testpip2.exe");

    Pipe = osl_createPipe( ustrPipeName, osl_Pipe_CREATE, 0 );

    if( !Pipe )
        fail( "unable to create Pipe.\n",
              osl_getLastPipeError(NULL));

    // starte client process
    ProcessError = osl_executeProcess( ustrExeName,
                                       NULL,
                                       0,
                                        osl_Process_NORMAL,
                                        0,
                                        NULL,
                                       NULL,
                                       0,
                                        NULL,
                                        &Process );

    if( ProcessError != osl_Process_E_None )
        fail( "unable to start client.\n", ProcessError );

    // wait for connection
    C1Pipe = osl_acceptPipe( Pipe );

    if( !C1Pipe )
        fail( "unable to connect to client.\n",
            osl_getLastPipeError( Pipe ));


    if( argc > 1 )
    {
        cp = argv[1];
        n  = strlen( cp ) + 1;
    }
    else
    {
        cp = szTestString;
        n  = sizeof(szTestString);
    }

    // sende TestString zum Client
    nChars = osl_sendPipe( C1Pipe, cp, n );

    if( nChars < 0 )
        fail( "unable to write on pipe.\n",
              osl_getLastPipeError( Pipe ) );

    // empfange Daten vom Server
    nChars = osl_receivePipe( C1Pipe, szBuffer, 256 );

    if( nChars < 0 )
        fail( "unable to read from pipe.\n",
              osl_getLastPipeError( C1Pipe ) );

    printf( "TestPipe Server: received data: %s.\n", szBuffer );

    // warte bis das Client-Programm sich beendet
    ProcessError = osl_joinProcess( Process );

    if( ProcessError != osl_Process_E_None )
        fail( "unable to wait for client.\n",
              ProcessError );

    // ermittle den Rckgabewert des Client-Programms
    ProcessInfo.Size = sizeof( ProcessInfo );

    ProcessError = osl_getProcessInfo( Process, osl_Process_EXITCODE, &ProcessInfo );

    if( ProcessError != osl_Process_E_None )
        fail( "unable to receive return value of client process.\n",
              ProcessError );

    if( ProcessInfo.Code != 0 )
        fail( "client aborted.\n", ProcessInfo.Code );

    // gib das Handle fuer den Client-Prozess frei
    osl_freeProcessHandle( Process );

    // schliesse die Pipes
    osl_destroyPipe( C1Pipe );
    osl_destroyPipe( Pipe );

    printf( "TestPipe Server: test passed.\n" );
    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
