/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inetdef.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:59:54 $
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

#ifndef _TOOLS_INETDEF_HXX
#define _TOOLS_INETDEF_HXX

//============================================================================
#if defined WNT
#define TOOLS_INETDEF_OS "Win32"
#elif defined OS2
#define TOOLS_INETDEF_OS "OS/2"
#elif defined UNX
#if defined AIX
#define TOOLS_INETDEF_OS "AIX"
#elif defined HPUX
#define TOOLS_INETDEF_OS "HP/UX"
#elif defined SOLARIS && defined SPARC
#define TOOLS_INETDEF_OS "Solaris Sparc"
#elif defined SOLARIS && defined INTEL
#define TOOLS_INETDEF_OS "Solaris x86"
#elif defined SCO
#define TOOLS_INETDEF_OS "SCO"
#elif defined NETBSD && defined X86
#define TOOLS_INETDEF_OS "NETBSD x86"
#elif defined NETBSD && defined ARM32
#define TOOLS_INETDEF_OS "NETBSD ARM32"
#elif defined NETBSD && defined SPARC
#define TOOLS_INETDEF_OS "NETBSD Sparc"
#elif defined LINUX && defined X86
#define TOOLS_INETDEF_OS "Linux"
#elif defined FREEBSD && defined X86
#define TOOLS_INETDEF_OS "FreeBSD/i386"
#elif defined FREEBSD && defined X86_64
#define TOOLS_INETDEF_OS "FreeBSD/amd64"
#elif defined SINIX
#define TOOLS_INETDEF_OS "SINIX"
#elif defined IRIX
#define TOOLS_INETDEF_OS "IRIX"
#else // AIX, HPUX, SOLARIS, ...
#define TOOLS_INETDEF_OS "Unix"
#endif // AIX, HPUX, SOLARIS, ...
#else // WNT, ...
#define TOOLS_INETDEF_OS "unknown OS"
#endif // WN, ...

#define TOOLS_INETDEF_PRODUCT "StarOffice/5.2"
#define TOOLS_INETDEF_MOZILLA "Mozilla/3.0"

#define INET_PRODUCTNAME TOOLS_INETDEF_PRODUCT " (" TOOLS_INETDEF_OS ")"
#define INET_DEF_CALLERNAME TOOLS_INETDEF_MOZILLA " (compatible; "           \
     TOOLS_INETDEF_PRODUCT "; " TOOLS_INETDEF_OS ")"

//============================================================================
// The following definitions seem obsolete and might get removed in future.

#define INET_PERS_CERT_HOMEPAGE "http://www.stardivision.de/certs.html"
#define INET_PERS_CERT_HOMEPAGE_INT                                          \
     "http://www.stardivision.de/certs/certs##.html"
    // the above definitions are only used in svx/source/options/optinet2.cxx

#if defined __RSC
#define INET_UNDEFINED 0
#define INET_NAME_RESOLVE_START 1
#define INET_NAME_RESOLVE_ERROR 2
#define INET_NAME_RESOLVE_SUCCESS 3
#define INET_CONNECT_START 4
#define INET_CONNECT_ERROR 5
#define INET_CONNECT_SUCCESS 6
#define INET_WRITE_START 7
#define INET_WRITE_STATUS 8
#define INET_WRITE_ERROR 9
#define INET_WRITE_SUCCESS 10
#define INET_READ_START 11
#define INET_READ_STATUS 12
#define INET_READ_ERROR 13
#define INET_READ_SUCCESS 14
#define INET_CLOSING_CONNECTION 15
#define INET_CONNECTION_CLOSED 16
#define INET_REQUEST_CANCELED 17
#define INET_CONNECTION_CANCELED 18
#define INET_SESSION_CANCELED 19
#define INET_AUTHENTICATION 20
#define INET_OFFLINE_ERROR 21
#define INET_PROXY_AUTHENTICATION 22
#endif // __RSC
    // the above definitions are only used in sfx2/source/doc/doc.src

#endif // _TOOLS_INETDEF_HXX

