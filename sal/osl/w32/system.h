/*************************************************************************
 *
 *  $RCSfile: system.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:23 $
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

#ifdef NDEBUG
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

#include <windows.h>

#ifdef GCC
#   include <winsock.h>
#else
#   include <winsock2.h>
#   include <wsipx.h>
#endif

#include <winbase.h>
#include <shlobj.h>

#ifndef NO_DEBUG_CRT
#   include <crtdbg.h>
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
#endif

#ifdef _DLL_
extern DWORD g_dwPlatformId;
#define IS_NT (g_dwPlatformId == VER_PLATFORM_WIN32_NT)
#else
extern DWORD GetPlatformId();
#define IS_NT (GetPlatformId() == VER_PLATFORM_WIN32_NT)
#endif


/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.9  2000/09/18 14:29:02  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.8  2000/05/22 15:29:08  obr
*    unicode changes channel/pipe
*
*    Revision 1.7  1999/10/27 15:13:37  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.6  1999/10/11 12:57:57  rt
*    gcc for windows: includes and defines
*
*    Revision 1.5  1998/09/18 15:33:44  rh
*    #56761# neue locations f?r cfg/ini files
*
*    Revision 1.4  1998/07/21 07:36:21  mh
*    dynamische C-Runtime
*
*    Revision 1.3  1998/02/16 19:35:00  rh
*    Cleanup of ports, integration of Size_t, features for process
*
*    Revision 1.2  1997/12/16 14:16:59  rh
*    Added locking and CR/LF conversion
*
*    Revision 1.1  1997/07/15 19:02:06  rh
*    system.h inserted
*
*************************************************************************/
