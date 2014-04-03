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

#ifndef _SOCKETHELPER_HXX_
#define _SOCKETHELPER_HXX_

#include <sal/types.h>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <rtl/ustring.h>

#include <osl/socket.hxx>
#include <osl/socket.h>

#include <osl/thread.hxx>

#include <osl/file.hxx>

#include <osl/mutex.hxx>
#include <osl/time.h>

#ifdef __cplusplus
extern "C"
{
#endif

// OS dependent declaration and includes

#if ( defined UNX )  //Unix

#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#endif

#ifndef _OSL_SOCKET_CONST_H_

#if ( defined WNT )                     // Windows
#include <winsock.h>
#include <string.h>
#endif

#endif

#ifdef __cplusplus
}
#endif

/** test output if SILENT_TEST is 0
*/
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

/** convert UString and OUString to std::string
*/
#define STD_STRING(s) (std::string((const char *)s.getStr()))

/** compare two OUString.
*/
sal_Bool compareUString( const ::rtl::OUString & ustr1, const ::rtl::OUString & ustr2 );
/** compare a OUString and an ASCII string.
*/
sal_Bool compareUString( const ::rtl::OUString & ustr, const sal_Char *astr );
/** compare two socket address.
*/
sal_Bool compareSocketAddr( const ::osl::SocketAddr & addr1 , const ::osl::SocketAddr & addr2  );
//char * oustring2char( const ::rtl::OUString & str );
/** print a UNI_CODE String. And also print some comments of the string.
*/
void printUString( const ::rtl::OUString & str, const char * msg = "" );
/** get the local host name.
    mindy: gethostbyname( "localhost" ), on Linux, it returns the hostname in /etc/hosts + domain name,
    if no entry in /etc/hosts, it returns "localhost" + domain name
*/
::rtl::OUString getHost( void );
/** get the full host name of the current processor, such as "aegean.prc.sun.com" --mindyliu
*/
::rtl::OUString getThisHostname( void );
/** get IP by name, search /etc/hosts first, then search from dns, fail return OUString("")
*/
::rtl::OUString getIPbyName( rtl::OString const& str_name );
/** get local ethernet IP
*/
::rtl::OUString getLocalIP( );
/** construct error message
*/
::rtl::OUString outputError( const ::rtl::OUString & returnVal, const ::rtl::OUString & rightVal, const sal_Char * msg = "");
void thread_sleep( sal_Int32 _nSec );
/** print Boolean value.
*/
void printBool( sal_Bool bOk );
/** print content of a ByteSequence.
*/
void printByteSequence_IP( const ::rtl::ByteSequence & bsByteSeq, sal_Int32 nLen );
/** convert an IP which is stored as a UString format to a ByteSequence array for later use.
*/
::rtl::ByteSequence UStringIPToByteSequence( ::rtl::OUString aUStr );
/** print a socket result name.
*/
void printSocketResult( oslSocketResult eResult );
/** if 4 parts of an IP addr are equal to specified values
*/
sal_Bool ifIpv4is( const ::rtl::ByteSequence Ipaddr, sal_Int8 seq1, sal_Int8 seq2, sal_Int8 seq3, sal_Int8 seq4 );
/** if the IP or hostname is  availble( alive )
*/
//sal_Bool ifAvailable( const char *  stringAddrOrHostName );
sal_Bool ifAvailable( rtl::OUString const&  strAddrOrHostName );
/*
class ClientSocketThread : public Thread
class ServerSocketThread : public Thread
class ValueCheckProvider
class ClientReadSocketThread : public Thread
class ServerWriteSocketThread : public Thread
class AcceptorThread : public Thread
class CloseSocketThread : public Thread

*/

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
