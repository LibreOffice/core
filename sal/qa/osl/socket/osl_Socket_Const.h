/*************************************************************************
*
 *  $RCSfile: osl_Socket_Const.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:11:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _OSL_SOCKET_CONST_H_
#define _OSL_SOCKET_CONST_H_

//------------------------------------------------------------------------
//------------------------------------------------------------------------
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifndef _OSL_SOCLET_HXX_
#include <osl/socket.hxx>
#endif

#ifndef _OSL_SOCKET_H_
#include <osl/socket.h>
#endif

#ifndef _OSL_THREAD_HXX
#include <osl/thread.hxx>
#endif

#ifndef _OSL_MUTEX_HXX
#include <osl/mutex.hxx>
#endif

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif


//------------------------------------------------------------------------
//------------------------------------------------------------------------


#ifdef __cplusplus
extern "C"
{
#endif

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
#else                                   // Windows
#   include <stdio.h>
#   include <windows.h>
#   include <winsock.h>
#   include <string.h>
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
#define IP_PORT_NETBIOS  139
#define IP_PORT_IMAP  143
#define IP_PORT_HTTPS  443
#define IP_PORT_HTTP2  8080
#define IP_PORT_MYPORT  8888
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
OSLTEST_DECLARE( HostIpInval,  "123.45.67.89" );
OSLTEST_DECLARE( HostNameInval,  "www.the_hostname_that_can_not_resolvable_to_an_IP_Address.com" );


//------------------------------------------------------------------------
// OS independent file definition
//------------------------------------------------------------------------
OSLTEST_DECLARE( NullURL,  "" );

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------


#endif /* _OSL_SOCKET_CONST_H_ */
