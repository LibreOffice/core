/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: system.h,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:22:20 $
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
#define _MAX_ENV    4096    /* maximum length of environment var */

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
    extern DWORD g_dwPlatformId;
    #define IS_NT (g_dwPlatformId == VER_PLATFORM_WIN32_NT)
#else
    extern DWORD GetPlatformId(void);
    #define IS_NT (GetPlatformId() == VER_PLATFORM_WIN32_NT)
#endif // #ifdef _DLL_


