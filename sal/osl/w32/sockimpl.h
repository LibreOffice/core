/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


