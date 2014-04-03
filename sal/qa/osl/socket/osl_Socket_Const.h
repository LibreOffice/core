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

#ifndef _OSL_SOCKET_CONST_H_
#define _OSL_SOCKET_CONST_H_

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

const char * pTestString1 = "test socket";
const char * pTestString2 = " Passed#OK";

#ifdef __cplusplus
extern "C"
{
#endif

#   include <stdio.h>

// OS dependent declaration and includes

#if ( defined UNX )     //Unix
#   include <unistd.h>
#   include <limits.h>
#   include <string.h>
#   include <math.h>
#   include <errno.h>
#   include <fcntl.h>
#   include <sys/stat.h>
#   include <sys/statfs.h>
#   include <sys/statvfs.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#       include <arpa/inet.h>
#endif
#if ( defined WNT )                     // Windows
#   include <winsock.h>
#   include <string.h>
#endif

// macro definition for the ASCII array/OUString declarations,
// we use p### for the ASCII array,
//        a### for the OUString,
//        n###Len for its length

#define OSLTEST_DECLARE( str_name, str_value ) \
    static const sal_Char  p##str_name[]        = str_value;  \
    static const sal_Int32 n##str_name##Len     = sizeof( p##str_name ) -1; \
    ::rtl::OUString a##str_name                 = rtl::OUString::createFromAscii( p##str_name )

// Ip version definition

#define IP_VER     4                   /// currently only IPv4 is considered.

// Hostnames.

// OSLTEST_DECLARE( HostName1,  "localhost" );
// OSLTEST_DECLARE( HostIp1,  "127.0.0.1" );
OSLTEST_DECLARE( HostName2,  "longshot.prc.sun.com" );
OSLTEST_DECLARE( HostIp2,  "129.158.217.202" );
OSLTEST_DECLARE( HostName3,  "deuce.prc.sun.com" );
OSLTEST_DECLARE( HostIp3,  "129.158.217.199" );
OSLTEST_DECLARE( HostName4,  "sceri.PRC.Sun.COM" );  //Beijing server for test
OSLTEST_DECLARE( HostIp4,  "129.158.217.107" );
OSLTEST_DECLARE( HostName5,  "koori.SFBay.Sun.COM" );  //"grande.Germany.Sun.COM" );  //Germany server for test
OSLTEST_DECLARE( HostIp5,  "10.5.32.20" );
OSLTEST_DECLARE( HostName6,  "crumple.SFBay.Sun.COM" );  //sfbay
OSLTEST_DECLARE( HostIp6,  "10.6.103.83" );
OSLTEST_DECLARE( HostIpInval,  "123.45.67.89" );    //this is valid ip number,but can not arrive --mindy
// OSLTEST_DECLARE( HostIpInval1,  "123.345.67.89" );   //this is real invalid ip number --mindy
OSLTEST_DECLARE( HostNameInval,  "www.the_hostname_that_can_not_resolvable_to_an_IP_Address.com" );
OSLTEST_DECLARE( HostIpZero,  "0.0.0.0" );

// OS independent file definition

OSLTEST_DECLARE( NullURL,  "" );

#ifdef __cplusplus
}
#endif

#endif /* _OSL_SOCKET_CONST_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
