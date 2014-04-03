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

#include <sal/types.h>
#include "sockethelper.hxx"
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#if OSL_DEBUG_LEVEL > 0
#   define SILENT_TEST 0
#else
#   define SILENT_TEST 1
#endif

#if SILENT_TEST
#   define t_print(...) { }
#else
#   define t_print printf
#endif

// Ip version definition

#define IP_VER     4               /// currently only IPv4 is considered.

// helper functions

/** compare two OUString.
*/
sal_Bool compareUString( const ::rtl::OUString & ustr1, const ::rtl::OUString & ustr2 )
{
    sal_Bool bOk = ustr1.equalsIgnoreAsciiCase( ustr2 );

    return bOk;
}

/** compare a OUString and an ASCII string.
*/
sal_Bool compareUString( const ::rtl::OUString & ustr, const sal_Char *astr )
{
    ::rtl::OUString ustr2 = rtl::OUString::createFromAscii( astr );
    sal_Bool bOk = ustr.equalsIgnoreAsciiCase( ustr2 );

    return bOk;
}

/** compare two socket address.
*/
sal_Bool compareSocketAddr( const ::osl::SocketAddr & addr1 , const ::osl::SocketAddr & addr2  )
{
    return ( ( sal_True == compareUString( addr1.getHostname( 0 ), addr2.getHostname( 0 ) ) ) && ( addr2.getPort( ) == addr2.getPort( ) ) );
}

/** print a UNI_CODE String. And also print some comments of the string.
*/
void printUString( const ::rtl::OUString & str, const char* msg)
{
#if SILENT_TEST
    (void)str;
    (void)msg;
#else
    t_print("#%s #printUString_u# ", msg );
    rtl::OString aString;
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    t_print("%s\n", aString.getStr( ) );
#endif
}

/** get the local host name.
    mindy: gethostbyname( "localhost" ), on Linux, it returns the hostname in /etc/hosts + domain name,
    if no entry in /etc/hosts, it returns "localhost" + domain name
*/
::rtl::OUString getHost( void )
{
    struct hostent *hptr;

    hptr = gethostbyname( "localhost" );
    OSL_ENSURE( hptr != NULL, "#In getHostname function, error on gethostbyname()" );
    ::rtl::OUString aUString = ::rtl::OUString::createFromAscii( (const sal_Char *) hptr->h_name );

    return aUString;
}

/** get the full host name of the current processor, such as "aegean.prc.sun.com" --mindyliu
*/
::rtl::OUString getThisHostname( void )
{
    ::rtl::OUString aUString;
#ifdef WNT
    struct hostent *hptr;
    hptr = gethostbyname( "localhost" );
    OSL_ENSURE( hptr != NULL, "#In getHostname function, error on gethostbyname()" );
    rtl::OString sHostname(hptr->h_name);
    aUString = ::rtl::OStringToOUString(sHostname, RTL_TEXTENCODING_ASCII_US);
#else
    char hostname[255];
    if (gethostname(hostname, 255) != 0) {
        OSL_FAIL( "#Error: gethostname failed."  );
    }

    struct hostent *hptr;
    //first search /ets/hosts, then search from dns
    hptr = gethostbyname( hostname);
    if ( hptr != NULL )
    {
    strcpy( hostname, hptr->h_name );
    }

    t_print("hostname is %s \n", hostname );
    rtl::OString sHostname( hostname );
    aUString = ::rtl::OStringToOUString( sHostname, RTL_TEXTENCODING_ASCII_US );
    aUString.getLength();
#endif
    return aUString;
}

/** get IP by name, search /etc/hosts first, then search from dns, fail return OUString("")
*/
::rtl::OUString getIPbyName( rtl::OString const& str_name )
{
    ::rtl::OUString aUString;
    struct hostent *hptr;
    //first search /ets/hosts, then search from dns
    hptr = gethostbyname( str_name.getStr());
    if ( hptr != NULL )
    {
        struct in_addr ** addrptr;
        addrptr = (struct in_addr **) hptr->h_addr_list ;
        //if there are more than one IPs on the same machine, we select one
        for (; *addrptr; addrptr++)
        {
            t_print("#Local IP Address: %s\n", inet_ntoa(**addrptr));
            aUString = ::rtl::OUString::createFromAscii( (sal_Char *) (inet_ntoa(**addrptr)) );
        }
    }
    return aUString;
}

/** get local ethernet IP
*/
::rtl::OUString getLocalIP( )
{
    char hostname[255];
    gethostname(hostname, 255);

    return getIPbyName( hostname );
}

/** construct error message
*/
::rtl::OUString outputError( const ::rtl::OUString & returnVal, const ::rtl::OUString & rightVal, const sal_Char * msg )
{
    ::rtl::OUString aUString;
    if ( returnVal.equals( rightVal ) )
        return aUString;
    aUString += ::rtl::OUString::createFromAscii(msg);
    aUString += ::rtl::OUString(":  the returned value is '");
    aUString += returnVal;
    aUString += ::rtl::OUString("', but the value should be '");
    aUString += rightVal;
    aUString += ::rtl::OUString("'.");
    return aUString;
}

/** wait _nSec seconds.
*/
void thread_sleep( sal_Int32 _nSec )
{
    /// print statement in thread process must use fflush() to force display.
    // printf("wait %d seconds. ", _nSec );
    // fflush(stdout);

#ifdef WNT               //Windows
    Sleep( _nSec * 100 );
#endif
#if ( defined UNX )  //Unix
    usleep(_nSec * 100000);
#endif
    // t_print("# done\n" );
}

/** print Boolean value.
*/
void printBool( sal_Bool bOk )
{
#if SILENT_TEST
    (void)bOk;
#else
    t_print("#printBool# " );
    t_print ("%s", (sal_True == bOk) ? "YES!\n" : "NO!\n");
#endif
}

/** print content of a ByteSequence.
*/
void printByteSequence_IP( const ::rtl::ByteSequence & bsByteSeq, sal_Int32 nLen )
{
#if SILENT_TEST
    (void)bsByteSeq;
    (void)nLen;
#else
    t_print("#ByteSequence is: " );
    for ( int i = 0; i < nLen; i++ ){
        if ( bsByteSeq[i] < 0 )
            t_print("%d ",  256 + bsByteSeq[i] );
        else
            t_print("%d ",  bsByteSeq[i] );
    }
    t_print(" .\n" );
#endif
}

/** convert an IP which is stored as a UString format to a ByteSequence array for later use.
*/
::rtl::ByteSequence UStringIPToByteSequence( ::rtl::OUString aUStr )
{

    rtl::OString aString = ::rtl::OUStringToOString( aUStr, RTL_TEXTENCODING_ASCII_US );
    const sal_Char *pChar = aString.getStr( ) ;
    sal_Char tmpBuffer[4];
    sal_Int32 nCharCounter = 0;
    ::rtl::ByteSequence bsByteSequence( IP_VER );
    sal_Int32 nByteSeqCounter = 0;

    for ( int i = 0; i < aString.getLength( ) + 1 ; i++ )
    {
        if ( ( *pChar != '.' ) && ( i !=aString.getLength( ) ) )
            tmpBuffer[nCharCounter++] = *pChar;
        else
        {
            tmpBuffer[nCharCounter] = '\0';
            nCharCounter = 0;
            bsByteSequence[nByteSeqCounter++] = static_cast<sal_Int8>(atoi( tmpBuffer ));
        }
        pChar++;
    }
    return bsByteSequence;
}

/** print a socket result name.
*/
void printSocketResult( oslSocketResult eResult )
{
    t_print("printSocketResult: " );
    if (!eResult)
    switch (eResult)
    {
        case osl_Socket_Ok:
            t_print("client connected\n");
            break;
        case osl_Socket_Error:
            t_print("got an error ... exiting\r\n\r\n" );
            break;
        case osl_Socket_TimedOut:
            t_print("timeout\n");
            break;
        case osl_Socket_Interrupted:
            t_print("interrupted\n");
            break;
        case osl_Socket_InProgress:
            t_print("in progress\n");
            break;
    default:
        t_print("unknown result\n");
        break;
    }
}

/** if 4 parts of an IP addr are equal to specified values
*/
sal_Bool ifIpv4is( const ::rtl::ByteSequence Ipaddr, sal_Int8 seq1, sal_Int8 seq2, sal_Int8 seq3, sal_Int8 seq4 )
{
    if ( ( Ipaddr[0]  == seq1 ) && ( Ipaddr[1] == seq2 ) && ( Ipaddr[2] == seq3 ) && ( Ipaddr[3] == seq4 ) )
        return sal_True;
    return sal_False;
}

sal_Bool ifAvailable( rtl::OUString const&  strAddrOrHostName )
{
    ::osl::ConnectorSocket aSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
    ::osl::SocketAddr aSocketAddr( strAddrOrHostName, 7 );

    if (! aSocketAddr.is())
    {
        aSocket.close();
        return sal_False;
    }

    aSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True;

    TimeValue *pTimeout;
    pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
    pTimeout->Seconds = 3;
    pTimeout->Nanosec = 0;

    oslSocketResult aResult = aSocket.connect( aSocketAddr, pTimeout );
    free( pTimeout );
    aSocket.close();
    if  ( aResult != osl_Socket_Ok )
    {
        t_print("Error: ");
        printSocketResult(aResult);
        t_print("\n");

        return sal_False;
    }
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
