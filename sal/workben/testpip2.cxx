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
