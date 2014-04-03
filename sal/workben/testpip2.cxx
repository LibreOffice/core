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

#include <osl/pipe.h>

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
    rtl_uString* ustrPipeName=0;
    sal_Int32 nChars;

    rtl_uString_newFromAscii(&ustrPipeName,pszPipeName);

    // erzeuge die Pipe
    Pipe = osl_createPipe( ustrPipeName, osl_Pipe_OPEN, 0 );

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
    osl_releasePipe( Pipe );

    printf( "TestPipe Client: test passed.\n" );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
