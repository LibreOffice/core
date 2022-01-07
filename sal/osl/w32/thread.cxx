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

#include "system.h"
#include "thread.hxx"

#include <comphelper/windowserrorstring.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <rtl/alloc.h>
#include <osl/time.h>
#include <osl/interlck.h>
#include <rtl/tencinfo.h>
#include <sal/log.hxx>
#include <systools/win32/comtools.hxx>

#include <errno.h>

namespace {

/**
    Thread-data structure hidden behind oslThread:
 */
typedef struct
{
    HANDLE              m_hThread;      /* OS-handle used for all thread-functions */
    DWORD               m_ThreadId;     /* identifier for this thread */
    sal_Int32           m_nTerminationRequested;
    oslWorkerFunction   m_WorkerFunction;
    void*               m_pData;

} osl_TThreadImpl;

}

static oslThread oslCreateThread(oslWorkerFunction pWorker, void* pThreadData, sal_uInt32 nFlags);

static DWORD WINAPI oslWorkerWrapperFunction(_In_ LPVOID pData)
{
    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(pData);

    /* Initialize COM - Multi Threaded Apartment (MTA) for all threads */
    sal::systools::CoInitializeGuard aGuard(COINIT_MULTITHREADED, false,
                                            sal::systools::CoInitializeGuard::WhenFailed::NoThrow);

    /* call worker-function with data */

    pThreadImpl->m_WorkerFunction(pThreadImpl->m_pData);

    return 0;
}

static oslThread oslCreateThread(oslWorkerFunction pWorker,
                                 void* pThreadData,
                                 sal_uInt32 nFlags)
{
    osl_TThreadImpl* pThreadImpl;

    /* alloc mem. for our internal data structure */
    pThreadImpl= static_cast<osl_TThreadImpl *>(malloc(sizeof(osl_TThreadImpl)));

    OSL_ASSERT(pThreadImpl);

    if ( pThreadImpl == nullptr )
    {
        return nullptr;
    }

    pThreadImpl->m_WorkerFunction= pWorker;
    pThreadImpl->m_pData= pThreadData;
    pThreadImpl->m_nTerminationRequested= 0;

    pThreadImpl->m_hThread= CreateThread(
                               nullptr,                     /* no security */
                               0,                           /* default stack-size */
                               oslWorkerWrapperFunction,    /* worker-function */
                               pThreadImpl,                 /* provide worker-function with data */
                               nFlags,                      /* start thread immediately or suspended */
                               &pThreadImpl->m_ThreadId);

    if(pThreadImpl->m_hThread == nullptr)
    {
        SAL_WARN("sal.osl", "CreateThread failed:" << WindowsErrorString(GetLastError()));

        /* create failed */
        free(pThreadImpl);
        return nullptr;
    }

    return pThreadImpl;
}

oslThread SAL_CALL osl_createThread(oslWorkerFunction pWorker,
                                    void* pThreadData)
{
    return oslCreateThread(pWorker, pThreadData, 0);
}

oslThread SAL_CALL osl_createSuspendedThread(oslWorkerFunction pWorker,
                                             void* pThreadData)
{
    return oslCreateThread(pWorker, pThreadData, CREATE_SUSPENDED);
}

oslThreadIdentifier SAL_CALL osl_getThreadIdentifier(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(Thread);

    if (pThreadImpl != nullptr)
        return static_cast<oslThreadIdentifier>(pThreadImpl->m_ThreadId);
    else
        return static_cast<oslThreadIdentifier>(GetCurrentThreadId());
}

void SAL_CALL osl_destroyThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(Thread);

    if (Thread == nullptr) /* valid ptr? */
    {
        /* thread already destroyed or not created */
        return;
    }

    /* !!!! _exitthreadex does _not_ call CloseHandle !!! */
    CloseHandle( pThreadImpl->m_hThread );

    /* free memory */
    free(Thread);
}

void SAL_CALL osl_resumeThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(Thread);

    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    ResumeThread(pThreadImpl->m_hThread);
}

void SAL_CALL osl_suspendThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(Thread);

    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    SuspendThread(pThreadImpl->m_hThread);
}

void SAL_CALL osl_setThreadPriority(oslThread Thread,
                           oslThreadPriority Priority)
{
    int winPriority;
    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(Thread);

    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    /*  map enum to WIN32 levels
        it would be faster and more elegant to preset
        the enums, but that would require an #ifdef in
        the exported header, which is not desired.
    */
    switch(Priority) {

    case osl_Thread_PriorityHighest:
        winPriority= THREAD_PRIORITY_HIGHEST;
        break;

    case osl_Thread_PriorityAboveNormal:
        winPriority= THREAD_PRIORITY_ABOVE_NORMAL;
        break;

    case osl_Thread_PriorityNormal:
        winPriority= THREAD_PRIORITY_NORMAL;
        break;

    case osl_Thread_PriorityBelowNormal:
        winPriority= THREAD_PRIORITY_BELOW_NORMAL;
        break;

    case osl_Thread_PriorityLowest:
        winPriority= THREAD_PRIORITY_LOWEST;
        break;

    case osl_Thread_PriorityUnknown:
        OSL_ASSERT(FALSE);      /* only fools try this...*/

        /* let release-version behave friendly */
        return;

    default:
        OSL_ASSERT(FALSE);      /* enum expanded, but forgotten here...*/

        /* let release-version behave friendly */
        return;
    }

    SetThreadPriority(pThreadImpl->m_hThread, winPriority);
}

oslThreadPriority SAL_CALL osl_getThreadPriority(const oslThread Thread)
{
    int winPriority;
    oslThreadPriority Priority;

    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(Thread);

    /* invalid arguments ?*/
    if(pThreadImpl==nullptr || pThreadImpl->m_hThread==nullptr)
    {
        return osl_Thread_PriorityUnknown;
    }

    winPriority=
        GetThreadPriority(pThreadImpl->m_hThread);

    if(winPriority == THREAD_PRIORITY_ERROR_RETURN)
    {
        return osl_Thread_PriorityUnknown;
    }

    /* map WIN32 priority to enum */
    switch(winPriority)
    {
    case THREAD_PRIORITY_TIME_CRITICAL:
    case THREAD_PRIORITY_HIGHEST:
        Priority= osl_Thread_PriorityHighest;
        break;

    case THREAD_PRIORITY_ABOVE_NORMAL:
        Priority= osl_Thread_PriorityAboveNormal;
        break;

    case THREAD_PRIORITY_NORMAL:
        Priority= osl_Thread_PriorityNormal;
        break;

    case THREAD_PRIORITY_BELOW_NORMAL:
        Priority= osl_Thread_PriorityBelowNormal;
        break;

    case THREAD_PRIORITY_IDLE:
    case THREAD_PRIORITY_LOWEST:
        Priority= osl_Thread_PriorityLowest;
        break;

    default:
        OSL_ASSERT(FALSE); /* WIN32 API changed, incorporate new prio-level! */

        /* release-version behaves friendly */
        Priority= osl_Thread_PriorityUnknown;
    }

    return Priority;
}

sal_Bool SAL_CALL osl_isThreadRunning(const oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(Thread);

    /* invalid arguments ?*/
    if(pThreadImpl==nullptr || pThreadImpl->m_hThread==nullptr)
    {
        return false;
    }

    return WaitForSingleObject(pThreadImpl->m_hThread, 0) != WAIT_OBJECT_0;
}

void SAL_CALL osl_joinWithThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(Thread);

    /* invalid arguments?*/
    if(pThreadImpl==nullptr || pThreadImpl->m_hThread==nullptr)
    {
        /* assume thread is not running */
        return;
    }

    WaitForSingleObject(pThreadImpl->m_hThread, INFINITE);
}

void SAL_CALL osl_waitThread(const TimeValue* pDelay)
{
    if (pDelay)
    {
        DWORD millisecs = pDelay->Seconds * 1000L + pDelay->Nanosec / 1000000L;

        Sleep(millisecs);
    }
}

void SAL_CALL osl_terminateThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(Thread);

    /* invalid arguments?*/
    if (pThreadImpl==nullptr || pThreadImpl->m_hThread==nullptr)
    {
        /* assume thread is not running */
        return;
    }

    osl_atomic_increment(&(pThreadImpl->m_nTerminationRequested));
}

sal_Bool SAL_CALL osl_scheduleThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= static_cast<osl_TThreadImpl*>(Thread);

    osl_yieldThread();

    /* invalid arguments?*/
    if (pThreadImpl==nullptr || pThreadImpl->m_hThread==nullptr)
    {
        /* assume thread is not running */
        return false;
    }

    return 0 == pThreadImpl->m_nTerminationRequested;
}

void SAL_CALL osl_yieldThread(void)
{
    Sleep(0);
}

static void impSetThreadDescription(char const * name) {
    // SetThreadDescription is only available since Windows 10 version 1607
    typedef HRESULT(WINAPI * TSetThreadDescription)(HANDLE, PCWSTR);
    static const auto pSetThreadDescription = reinterpret_cast<TSetThreadDescription>(
        GetProcAddress(GetModuleHandleA("KernelBase.dll"), "SetThreadDescription"));
    if (pSetThreadDescription)
    {
        if (const int nReqCCh = MultiByteToWideChar(CP_ACP, 0, name, -1, nullptr, 0))
        {
            if (PWSTR wStr = static_cast<PWSTR>(malloc(nReqCCh * sizeof(WCHAR))))
            {
                if (MultiByteToWideChar(CP_ACP, 0, name, -1, wStr, nReqCCh))
                    pSetThreadDescription(GetCurrentThread(), wStr);
                free(wStr);
            }
        }
    }
}

void SAL_CALL osl_setThreadName(char const * name) {
    /* See < https://docs.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code >: */
#pragma pack(push, 8)
    struct {
        DWORD dwType = 0x1000;
        LPCSTR szName;
        DWORD dwThreadID = DWORD(-1);
        DWORD dwFlags = 0;
    } info;
#pragma pack(pop)
    info.szName = name;
    __try {
        RaiseException(
            0x406D1388, 0, sizeof info / sizeof (ULONG_PTR),
            reinterpret_cast<ULONG_PTR *>(&info));
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    impSetThreadDescription(name);
}

namespace {

typedef struct TLS_
{
    DWORD                           dwIndex;
    oslThreadKeyCallbackFunction    pfnCallback;
    struct TLS_                     *pNext, *pPrev;
} TLS, *PTLS;

PTLS g_pThreadKeyList = nullptr;
osl::Mutex& getThreadKeyListMutex()
{
    static osl::Mutex g_ThreadKeyListMutex;
    return g_ThreadKeyListMutex;
}

}

static void AddKeyToList( PTLS pTls )
{
    if ( pTls )
    {
        osl::MutexGuard aGuard(getThreadKeyListMutex());

        pTls->pNext = g_pThreadKeyList;
        pTls->pPrev = nullptr;

        if ( g_pThreadKeyList )
            g_pThreadKeyList->pPrev = pTls;

        g_pThreadKeyList = pTls;
    }
}

static void RemoveKeyFromList( PTLS pTls )
{
    if ( pTls )
    {
        osl::MutexGuard aGuard(getThreadKeyListMutex());
        if ( pTls->pPrev )
            pTls->pPrev->pNext = pTls->pNext;
        else
        {
            OSL_ASSERT( pTls == g_pThreadKeyList );
            g_pThreadKeyList = pTls->pNext;
        }

        if ( pTls->pNext )
            pTls->pNext->pPrev = pTls->pPrev;
    }
}

void osl_callThreadKeyCallbackOnThreadDetach(void)
{
    PTLS    pTls;

    osl::MutexGuard aGuard(getThreadKeyListMutex());
    pTls = g_pThreadKeyList;
    while ( pTls )
    {
        if ( pTls->pfnCallback )
        {
            void    *pValue = TlsGetValue( pTls->dwIndex );

            if ( pValue )
                pTls->pfnCallback( pValue );
        }

        pTls = pTls->pNext;
    }
}

oslThreadKey SAL_CALL osl_createThreadKey(oslThreadKeyCallbackFunction pCallback)
{
    PTLS    pTls = static_cast<PTLS>(malloc( sizeof(TLS) ));

    if ( pTls )
    {
        pTls->pfnCallback = pCallback;
        if ( DWORD(-1) == (pTls->dwIndex = TlsAlloc()) )
        {
            free( pTls );
            pTls = nullptr;
        }
        else
            AddKeyToList( pTls );
    }

    return pTls;
}

void SAL_CALL osl_destroyThreadKey(oslThreadKey Key)
{
    if (Key != nullptr)
    {
        PTLS    pTls = static_cast<PTLS>(Key);

        RemoveKeyFromList( pTls );
        TlsFree( pTls->dwIndex );
        free( pTls );
    }
}

void* SAL_CALL osl_getThreadKeyData(oslThreadKey Key)
{
    if (Key != nullptr)
    {
        PTLS    pTls = static_cast<PTLS>(Key);

        return TlsGetValue( pTls->dwIndex );
    }

    return nullptr;
}

sal_Bool SAL_CALL osl_setThreadKeyData(oslThreadKey Key, void *pData)
{
    if (Key != nullptr)
    {
        PTLS    pTls = static_cast<PTLS>(Key);
        void*   pOldData = nullptr;
        bool    fSuccess;

        if ( pTls->pfnCallback )
            pOldData = TlsGetValue( pTls->dwIndex );

        fSuccess = TlsSetValue( pTls->dwIndex, pData );

        if ( fSuccess && pTls->pfnCallback && pOldData )
            pTls->pfnCallback( pOldData );

        return fSuccess;
    }

    return false;
}

namespace
{
rtl_TextEncoding& getThreadTextEncodingImpl()
{
    static thread_local rtl_TextEncoding s_enc = rtl_getTextEncodingFromWindowsCodePage(GetACP());
    return s_enc;
}
}

rtl_TextEncoding SAL_CALL osl_getThreadTextEncoding(void) { return getThreadTextEncodingImpl(); }

rtl_TextEncoding SAL_CALL osl_setThreadTextEncoding( rtl_TextEncoding Encoding )
{
    rtl_TextEncoding oldEncoding = getThreadTextEncodingImpl();
    getThreadTextEncodingImpl() = Encoding;
    return oldEncoding;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
