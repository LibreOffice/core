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

#include <osl/pipe.h>
#include <osl/socket.h>
#include <osl/interlck.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*oslCloseCallback) (void*);

#if defined(LINUX) || defined(FREEBSD) || defined(NETBSD)
#define CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT 1
#else
#define CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT 0
#endif

struct oslSocketImpl {
    int                 m_Socket;
    int                 m_nLastError;
    oslCloseCallback    m_CloseCallback;
    void*               m_CallbackArg;
    oslInterlockedCount m_nRefCount;
#if CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT
    sal_Bool            m_bIsAccepting;
    sal_Bool            m_bIsInShutdown;
#endif
};

struct oslSocketAddrImpl
{
    sal_Int32 m_nRefCount;
    union
    {
        struct sockaddr m_sockaddr;
        struct sockaddr_in m_sockaddr_in;
    };
};

struct oslPipeImpl {
    int  m_Socket;
    sal_Char m_Name[PATH_MAX + 1];
    oslInterlockedCount m_nRefCount;
    sal_Bool m_bClosed;
#if CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT
    sal_Bool m_bIsAccepting;
    sal_Bool m_bIsInShutdown;
#endif
};

oslSocket __osl_createSocketImpl(int Socket);
void __osl_destroySocketImpl(oslSocket pImpl);

#ifdef __cplusplus
}
#endif

#endif

