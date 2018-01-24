/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include <stdio.h>

#include <sal/main.h>

#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>

using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OString;

SAL_IMPLEMENT_MAIN()
{
    // string concatenation

    sal_Int32 n = 42;
    double pi = 3.14159;

    // give it an initial size, should be a good guess.
    // stringbuffer extends if necessary
    OUStringBuffer buf( 128 );

    // append an ascii string
    buf.append( "pi ( here " );

    // numbers can be simply appended
    buf.append( pi );

    // lets the compiler count the stringlength, so this is more efficient than
    // the above appendAscii call, where length of the string must be calculated at
    // runtime
    buf.append( " ) multiplied with " );
    buf.append( n );
    buf.append( " gives " );
    buf.append( (double)( n * pi ) );
    buf.append( "." );

    // now transfer the buffer into the string.
    // afterwards buffer is empty and may be reused again !
    OUString string = buf.makeStringAndClear();

    // I could of course also used the OStringBuffer directly
    OString oString = rtl::OUStringToOString( string , RTL_TEXTENCODING_ASCII_US );

    // just to print something
    printf( "%s\n" ,oString.getStr() );

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
