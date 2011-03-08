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

#ifndef _OSL_SOCKETIMPL_H_
#define _OSL_SOCKETIMPL_H_

#include <osl/socket.h>
#include <osl/interlck.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* oslSocketImpl */
/*****************************************************************************/
#define OSL_SOCKET_FLAGS_NONBLOCKING    0x0001

typedef void* (SAL_CALL * oslCloseCallback) (void*);

struct oslSocketImpl {
    oslInterlockedCount m_nRefCount;
    SOCKET              m_Socket;
    int                 m_Flags;
    oslCloseCallback    m_CloseCallback;
    void*               m_CallbackArg;
};

struct oslSocketAddrImpl
{
    struct sockaddr m_sockaddr;
    oslInterlockedCount m_nRefCount;
};

oslSocket __osl_createSocketImpl(SOCKET Socket);
void __osl_destroySocketImpl(oslSocket pImpl);

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
