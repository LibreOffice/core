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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <osl/pipe.h>
#include <osl/process.h>
#include <rtl/ustring.h>

// clear Name for the Pipe
const char pszPipeName[] = "TestPipe";
const char szTestString[] = "This is a test";
char       szBuffer[256];

const char *  cp;
size_t  n;
sal_Int32 nChars;

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
 * Test the Pipe-Implementation in osl
 */

int main (int argc, const char *argv[])
{
    // create the Pipe
    rtl_uString* ustrPipeName=0;
    rtl_uString* ustrExeName=0;

    rtl_uString_newFromAscii(&ustrPipeName,pszPipeName);
    rtl_uString_newFromAscii(&ustrExeName, "//./tmp/testpip2.exe");

    Pipe = osl_createPipe( ustrPipeName, osl_Pipe_CREATE, 0 );

    if( !Pipe )
        fail( "unable to create Pipe.\n",
              osl_getLastPipeError(NULL));

    // start client process
    ProcessError = osl_executeProcess( ustrExeName,
                                       NULL,
                                       0,
                                        osl_Process_NORMAL,
                                        0,
                                       NULL,
                                       NULL,
                                        0,
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

    // send TestString to Client
    nChars = osl_sendPipe( C1Pipe, cp, n );

    if( nChars < 0 )
        fail( "unable to write on pipe.\n",
              osl_getLastPipeError( Pipe ) );

    // receive data from the server
    nChars = osl_receivePipe( C1Pipe, szBuffer, 256 );

    if( nChars < 0 )
        fail( "unable to read from pipe.\n",
              osl_getLastPipeError( C1Pipe ) );

    printf( "TestPipe Server: received data: %s.\n", szBuffer );

    // wait until the client-program terminates
    ProcessError = osl_joinProcess( Process );

    if( ProcessError != osl_Process_E_None )
        fail( "unable to wait for client.\n",
              ProcessError );

    // investigate the return-value of the client-program
    ProcessInfo.Size = sizeof( ProcessInfo );

    ProcessError = osl_getProcessInfo( Process, osl_Process_EXITCODE, &ProcessInfo );

    if( ProcessError != osl_Process_E_None )
        fail( "unable to receive return value of client process.\n",
              ProcessError );

    if( ProcessInfo.Code != 0 )
        fail( "client aborted.\n", ProcessInfo.Code );

    // give the handle for the free client-process
    osl_freeProcessHandle( Process );

    // close the pipes
    osl_releasePipe( C1Pipe );
    osl_releasePipe( Pipe );

    printf( "TestPipe Server: test passed.\n" );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
