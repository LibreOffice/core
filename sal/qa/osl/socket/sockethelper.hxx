/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sockethelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:36:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _SOCKETHELPER_HXX_
#define _SOCKETHELPER_HXX_

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

#ifndef _OSL_FILE_HXX
#include <osl/file.hxx>
#endif

#ifndef _OSL_MUTEX_HXX
#include <osl/mutex.hxx>
#endif

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------
// OS dependent declaration and includes
//------------------------------------------------------------------------
#if ( defined UNX ) || ( defined OS2 )  //Unix

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
#include <tools/prewin.h>
// #include <windows.h>
#include <winsock.h>
#include <string.h>
#include <tools/postwin.h>
#endif

#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

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
