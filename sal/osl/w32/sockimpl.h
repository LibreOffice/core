/*************************************************************************
 *
 *  $RCSfile: sockimpl.h,v $
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

#ifndef _OSL_SOCKETIMPL_H_
#define _OSL_SOCKETIMPL_H_

#include <osl/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* oslSocketImpl */
/*****************************************************************************/
#define OSL_SOCKET_FLAGS_NONBLOCKING    0x0001

typedef void* (SAL_CALL * oslCloseCallback) (void*);

typedef struct _oslSocketImpl {
    SOCKET              m_Socket;
    int                 m_Flags;
    oslCloseCallback    m_CloseCallback;
    void*               m_CallbackArg;
} oslSocketImpl;

oslSocketImpl* __osl_createSocketImpl(SOCKET Socket);
void __osl_destroySocketImpl(oslSocketImpl *pImpl);

/*****************************************************************************/
/* oslSocketDialupImpl */
/*****************************************************************************/
#define INTERNET_MODULE_NAME "wininet.dll"

#define INTERNET_CONNECTION_MODEM  0x00000001L
#define INTERNET_CONNECTION_LAN    0x00000002L
#define INTERNET_CONNECTION_HANGUP 0x80000000L

typedef DWORD (WINAPI *INTERNETATTEMPTCONNECT) (
    DWORD dwReserved);
typedef BOOL (WINAPI *INTERNETAUTODIAL) (
    DWORD dwFlags, DWORD dwReserved);
typedef BOOL (WINAPI *INTERNETAUTODIALHANGUP) (
    DWORD dwReserved);
typedef BOOL (WINAPI *INTERNETGETCONNECTEDSTATE) (
    LPDWORD lpdwFlags, DWORD dwReserved);

typedef struct osl_socket_dialup_impl_st
{
    CRITICAL_SECTION          m_hMutex;
    HINSTANCE                 m_hModule;
    INTERNETATTEMPTCONNECT    m_pfnAttemptConnect;
    INTERNETAUTODIAL          m_pfnAutodial;
    INTERNETAUTODIALHANGUP    m_pfnAutodialHangup;
    INTERNETGETCONNECTEDSTATE m_pfnGetConnectedState;
    DWORD                     m_dwFlags;
} oslSocketDialupImpl;

static oslSocketDialupImpl* __osl_createSocketDialupImpl (void);
static void __osl_initSocketDialupImpl (oslSocketDialupImpl *pImpl);
static void __osl_destroySocketDialupImpl (oslSocketDialupImpl *pImpl);

static sal_Bool __osl_querySocketDialupImpl (void);
static sal_Bool __osl_attemptSocketDialupImpl (void);

/*****************************************************************************/
/* The End */
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif


/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.10  2000/09/18 14:29:02  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.9  2000/05/29 16:34:15  hro
*    SRC591: Explicite SAL_CALL calling convention
*
*    Revision 1.8  1999/10/27 15:13:37  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.7  1999/02/16 20:09:23  mhu
*    Fixed WININET calling convention (WINAPI instead of __cdecl) (#49995#).
*
*    Revision 1.6  1999/02/13 18:18:17  mhu
*    Added support for modem dialup handling (oslSocketDialupImpl) (#49995#).
*
*    Revision 1.5  1998/03/06 15:42:12  rh
*    Added signal handling
*
*    Revision 1.4  1997/11/28 15:34:42  ts
*    bugfix: bei weitergereichten Sockets wird der kindprozess solange blockiert, bis der vaterprozess diese Sockets closed
*
*    Revision 1.3  1997/09/04 09:51:47  rh
*    Blockmode
*
*    Revision 1.2  1997/07/15 19:34:39  rh
*    inserted #ifdefs
*
*    Revision 1.1  1997/07/15 19:24:13  rh
*    *** empty log message ***
*
*************************************************************************/
