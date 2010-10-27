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

#if OSL_DEBUG_LEVEL == 0
#    define NO_DEBUG_CRT
#endif

#ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x0400
#   define _CTYPE_DISABLE_MACROS /* wg. dynamischer C-Runtime MH */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <malloc.h>
#include <limits.h>
#include <process.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <share.h>
#include <direct.h>

/* Must define this else build breaks because Winsock2.h
    includes Windows.h and without WIN32_LEAN_AND_MEAN
    also includes mswsock.h which needs a forward typedef
    of SOCKET ...
*/
#define WIN32_LEAN_AND_MEAN

#ifdef GCC
    // windows.h includes winsock2.h
    // if _WIN32_WINNT > 0x0400
    // so someone cannot include winsock.h
    // at the same time without patching
    // windows.h
    #include <windows.h>
#ifdef __MINGW32__
    #include <winsock2.h>
    #include <ws2tcpip.h>
#endif
    #include <shlobj.h>
    #ifndef NO_DEBUG_CRT
        #include <crtdbg.h>
    #endif
#else
    // winsock2.h includes windows.h
    #pragma warning(push,1) /* disable warnings within system headers */
    #pragma warning(disable:4917)
    #include <winsock2.h>
    #include <wsipx.h>
    #include <shlobj.h>
    #ifndef NO_DEBUG_CRT
        #include <crtdbg.h>
    #endif
    #pragma warning(pop)
#endif

#define _MAX_CMD    4096    /* maximum length of commandline     */
/* #define _MAX_ENV 4096     maximum length of environment var (isn't used anywhere) */

#ifdef GCC
    #   ifndef SA_FAMILY_DECL
    #       define SA_FAMILY_DECL short sa_family
    #   endif

    typedef struct sockaddr_ipx {
        SA_FAMILY_DECL;
        char sa_netnum[4];
        char sa_nodenum[6];
        unsigned short sa_socket;
    } SOCKADDR_IPX;

    #   define NSPROTO_IPX      1000
    #   define NSPROTO_SPX      1256
    #   define NSPROTO_SPXII    1257
#endif // #ifdef GCC

#ifdef _DLL_

#ifdef __cplusplus
    extern "C" DWORD g_dwPlatformId;
#else
    extern DWORD g_dwPlatformId;
#endif // #ifdef __cplusplus

    #define IS_NT (g_dwPlatformId == VER_PLATFORM_WIN32_NT)
#else

#ifdef __cplusplus
    extern "C" DWORD GetPlatformId(void);
#else
    extern DWORD GetPlatformId(void);
#endif // #ifdef __cplusplus

    #define IS_NT (GetPlatformId() == VER_PLATFORM_WIN32_NT)
#endif // #ifdef _DLL_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
