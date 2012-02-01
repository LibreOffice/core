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



#include <stdio.h>

#include <sal/main.h>

#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>

using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OString;

SAL_IMPLEMENT_MAIN()
{
    // string concatination

    sal_Int32 n = 42;
    double pi = 3.14159;

    // give it an initial size, should be a good guess.
    // stringbuffer extends if necessary
    OUStringBuffer buf( 128 );

    // append an ascii string
    buf.appendAscii( "pi ( here " );

    // numbers can be simply appended
    buf.append( pi );

    // lets the compiler count the stringlength, so this is more efficient than
    // the above appendAscii call, where length of the string must be calculated at
    // runtime
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" ) multiplied with " ) );
    buf.append( n );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" gives ") );
    buf.append( (double)( n * pi ) );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "." ) );

    // now transfer the buffer into the string.
    // afterwards buffer is empty and may be reused again !
    OUString string = buf.makeStringAndClear();

    // I could of course also used the OStringBuffer directly
    OString oString = rtl::OUStringToOString( string , RTL_TEXTENCODING_ASCII_US );

    // just to print something
    printf( "%s\n" ,oString.getStr() );

    return 0;
}

