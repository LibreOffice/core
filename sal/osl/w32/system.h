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

#if OSL_DEBUG_LEVEL <= 3
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

#ifdef __GNUC__
    // windows.h includes winsock2.h
    // if _WIN32_WINNT > 0x0400
    // so someone cannot include winsock.h
    // at the same time without patching
    // windows.h
    #include <windows.h>
    #include <winsock2.h>
    #ifdef __cplusplus
    extern "C" {
    #endif
    #include <ws2tcpip.h>
    #ifdef __cplusplus
    }
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

#ifdef __GNUC__
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
#endif // #ifdef __GNUC__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
