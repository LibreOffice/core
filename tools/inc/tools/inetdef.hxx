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
#elif defined SOLARIS && defined SPARC
#define TOOLS_INETDEF_OS "Solaris Sparc"
#elif defined SOLARIS && defined INTEL
#define TOOLS_INETDEF_OS "Solaris x86"
#elif defined SCO
#define TOOLS_INETDEF_OS "SCO"
#elif defined NETBSD && defined X86
#define TOOLS_INETDEF_OS "NetBSD/i386"
#elif defined NETBSD && defined X86_64
#define TOOLS_INETDEF_OS "NetBSD/amd64"
#elif defined NETBSD && defined ARM32
#define TOOLS_INETDEF_OS "NetBSD/arm"
#elif defined NETBSD && defined SPARC
#define TOOLS_INETDEF_OS "NetBSD/sparc"
#elif defined LINUX && defined X86
#define TOOLS_INETDEF_OS "Linux"
#elif defined FREEBSD && defined X86
#define TOOLS_INETDEF_OS "FreeBSD/i386"
#elif defined FREEBSD && defined X86_64
#define TOOLS_INETDEF_OS "FreeBSD/amd64"
#elif defined OPENBSD && defined X86
#define TOOLS_INETDEF_OS "OpenBSD/i386"
#elif defined OPENBSD && defined X86_64
#define TOOLS_INETDEF_OS "OpenBSD/amd64"
#elif defined DRAGONFLY && defined X86
#define TOOLS_INETDEF_OS "DragonFly/i386"
#elif defined DRAGONFLY && defined X86_64
#define TOOLS_INETDEF_OS "DragonFly/amd64"
#else // AIX, SOLARIS, ...
#define TOOLS_INETDEF_OS "Unix"
#endif // AIX, SOLARIS, ...
#else // WNT, ...
#define TOOLS_INETDEF_OS "unknown OS"
#endif // WN, ...

//============================================================================
// The following definitions seem obsolete and might get removed in future.

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
