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

#ifndef _OSL_SOCKET_CONST_H_
#define _OSL_SOCKET_CONST_H_

#include <sal/types.h>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <rtl/ustring.h>

#ifndef _OSL_SOCLET_HXX_
#include <osl/socket.hxx>
#endif
#include <osl/socket.h>

#ifndef _OSL_THREAD_HXX
#include <osl/thread.hxx>
#endif

#ifndef _OSL_FILE_HXX
#include <osl/file.hxx>
#endif

#ifndef _OSL_MUTEX_HXX
#include <osl/mutex.hxx>
#endif
#include <osl/time.h>

const char * pTestString1 = "test socket";
const char * pTestString2 = " Passed#OK";

#ifdef __cplusplus
extern "C"
{
#endif

#   include <stdio.h>
//------------------------------------------------------------------------
// OS dependent declaration and includes
//------------------------------------------------------------------------
#if ( defined UNX ) || ( defined OS2 )  //Unix
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
#include <prewin.h>
#   include <winsock.h>
#   include <string.h>
#include <postwin.h>
#endif


//------------------------------------------------------------------------
// macro definition for the ASCII array/OUString declarations,
// we use p### for the ASCII array,
//        a### for the OUString,
//        n###Len for its length
//------------------------------------------------------------------------

#define OSLTEST_DECLARE( str_name, str_value ) \
    static const sal_Char  p##str_name[]        = str_value;  \
    static const sal_Int32 n##str_name##Len     = sizeof( p##str_name ) -1; \
    ::rtl::OUString a##str_name                 = rtl::OUString::createFromAscii( p##str_name )


//------------------------------------------------------------------------
// Ip version definition
//------------------------------------------------------------------------
#define IP_VER     4                   /// currently only IPv4 is considered.

//------------------------------------------------------------------------
// Ip port definition
//------------------------------------------------------------------------
#define IP_PORT_ZERO    0
#define IP_PORT_FTP    21
#define IP_PORT_TELNET 23
#define IP_PORT_SMTP 25
#define IP_PORT_GOPHER 70
#define IP_PORT_HTTP1  80
#define IP_PORT_POP3  110
#define IP_PORT_NETBIOS_DGM  138
#define IP_PORT_NETBIOS  139
#define IP_PORT_IMAP  143
#define IP_PORT_HTTPS  443
#define IP_PORT_HTTP2  8080
/**reference: http://www.iana.org/assignments/port-numbers */
#define IP_PORT_MYPORT  8881    //8888
#define IP_PORT_MYPORT1  8882   //8889
#define IP_PORT_MYPORT2  8883   //8890
#define IP_PORT_MYPORT3  8884   //8891
#define IP_PORT_MYPORT4  8885   //8892
#define IP_PORT_MYPORT5  8886   //8893
#define IP_PORT_MYPORT6  8887   //8894
#define IP_PORT_MYPORT7  8895
#define IP_PORT_MYPORT8  8896
#define IP_PORT_MYPORT9  8897
#define IP_PORT_MYPORT10  8898

#define IP_PORT_TMP    9999
#define IP_PORT_INVAL  99999

//------------------------------------------------------------------------
// service definitions.
//------------------------------------------------------------------------
OSLTEST_DECLARE( ServiceFTP,  "ftp" );
OSLTEST_DECLARE( ServiceTELNET,  "telnet" );
OSLTEST_DECLARE( ServiceGOPHER,  "gopher" );
OSLTEST_DECLARE( ServiceIMAP,  "imap" );
OSLTEST_DECLARE( ServiceHTTPS,  "https" );
OSLTEST_DECLARE( ServiceNETBIOS,  "netbios-dgm" );

//------------------------------------------------------------------------
// protocol definitions.
//------------------------------------------------------------------------
OSLTEST_DECLARE( ProtocolTCP,  "tcp" );
OSLTEST_DECLARE( ProtocolUDP,  "udp" );

//------------------------------------------------------------------------
// Hostnames.
//------------------------------------------------------------------------
OSLTEST_DECLARE( HostName1,  "localhost" );
OSLTEST_DECLARE( HostIp1,  "127.0.0.1" );
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
OSLTEST_DECLARE( HostIpInval1,  "123.345.67.89" );  //this is real invalid ip number --mindy
OSLTEST_DECLARE( HostNameInval,  "www.the_hostname_that_can_not_resolvable_to_an_IP_Address.com" );
OSLTEST_DECLARE( HostIpZero,  "0.0.0.0" );

//------------------------------------------------------------------------
// OS independent file definition
//------------------------------------------------------------------------
OSLTEST_DECLARE( NullURL,  "" );

#ifdef __cplusplus
}
#endif

#endif /* _OSL_SOCKET_CONST_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
