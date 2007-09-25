/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sockimpl.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-25 09:51:05 $
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

#ifndef _OSL_SOCKETIMPL_H_
#define _OSL_SOCKETIMPL_H_

#include <osl/pipe.h>
#include <osl/socket.h>
#include <osl/interlck.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*oslCloseCallback) (void*);

struct oslSocketImpl {
    int                 m_Socket;
    int                 m_nLastError;
    oslCloseCallback    m_CloseCallback;
    void*               m_CallbackArg;
    oslInterlockedCount m_nRefCount;
#if defined(LINUX)
    sal_Bool            m_bIsAccepting;
    sal_Bool            m_bIsInShutdown;
#endif
};

struct oslSocketAddrImpl
{
    sal_Int32 m_nRefCount;
    struct sockaddr m_sockaddr;
};

/*
struct oslPipeImpl {
    int  m_Socket;
    sal_Char m_Name[PATH_MAX + 1];
    oslInterlockedCount m_nRefCount;
    sal_Bool m_bClosed;
#if defined(LINUX)
    sal_Bool m_bIsAccepting;
    sal_Bool m_bIsInShutdown;
#endif
};
*/

oslSocket __osl_createSocketImpl(int Socket);
void __osl_destroySocketImpl(oslSocket pImpl);

#ifdef __cplusplus
}
#endif

#endif

