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

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/alloc.h>
#include <osl/time.h>
#include <osl/interlck.h>
#include <rtl/tencinfo.h>

/*
    Thread-data structure hidden behind oslThread:
*/
typedef struct _osl_TThreadImpl
{
    HANDLE              m_hThread;      /* OS-handle used for all thread-functions */
    unsigned            m_ThreadId;     /* identifier for this thread */
    sal_Int32           m_nTerminationRequested;
    oslWorkerFunction   m_WorkerFunction;
    void*               m_pData;

} osl_TThreadImpl;

#define THREADIMPL_FLAGS_TERMINATE  0x0001

static unsigned __stdcall oslWorkerWrapperFunction(void* pData);
static oslThread oslCreateThread(oslWorkerFunction pWorker, void* pThreadData, sal_uInt32 nFlags);

/*****************************************************************************/
/* oslWorkerWrapperFunction */
/*****************************************************************************/
static unsigned __stdcall oslWorkerWrapperFunction(void* pData)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)pData;

    /* Initialize COM */

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    /* call worker-function with data */

    pThreadImpl->m_WorkerFunction(pThreadImpl->m_pData);

    CoUninitialize();

    return (0);
}

/*****************************************************************************/
/* oslCreateThread */
/*****************************************************************************/
static oslThread oslCreateThread(oslWorkerFunction pWorker,
                                 void* pThreadData,
                                 sal_uInt32 nFlags)
{
    osl_TThreadImpl* pThreadImpl;

    /* alloc mem. for our internal data structure */
    pThreadImpl= malloc(sizeof(osl_TThreadImpl));

    OSL_ASSERT(pThreadImpl);

    if ( pThreadImpl == 0 )
    {
        return 0;
    }

    pThreadImpl->m_WorkerFunction= pWorker;
    pThreadImpl->m_pData= pThreadData;
    pThreadImpl->m_nTerminationRequested= 0;

    pThreadImpl->m_hThread=
        (HANDLE)_beginthreadex(NULL,                        /* no security */
                               0,                           /* default stack-size */
                               oslWorkerWrapperFunction,    /* worker-function */
                               pThreadImpl,                 /* provide worker-function with data */
                               nFlags,                      /* start thread immediately or suspended */
                               &pThreadImpl->m_ThreadId);

    if(pThreadImpl->m_hThread == 0)
    {
        /* create failed */
        free(pThreadImpl);
        return 0;
    }

    return (oslThread)pThreadImpl;
}

/*****************************************************************************/
/* osl_createThread */
/*****************************************************************************/
oslThread SAL_CALL osl_createThread(oslWorkerFunction pWorker,
                                    void* pThreadData)
{
    return oslCreateThread(pWorker, pThreadData, 0);
}

/*****************************************************************************/
/* osl_createSuspendedThread */
/*****************************************************************************/
oslThread SAL_CALL osl_createSuspendedThread(oslWorkerFunction pWorker,
                                             void* pThreadData)
{
    return oslCreateThread(pWorker, pThreadData, CREATE_SUSPENDED);
}

/*****************************************************************************/
/* osl_getThreadIdentifier */
/*****************************************************************************/
oslThreadIdentifier SAL_CALL osl_getThreadIdentifier(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    if (pThreadImpl != NULL)
        return ((oslThreadIdentifier)pThreadImpl->m_ThreadId);
    else
        return ((oslThreadIdentifier)GetCurrentThreadId());
}

/*****************************************************************************/
/* osl_destroyThread */
/*****************************************************************************/
void SAL_CALL osl_destroyThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    if (Thread == 0) /* valid ptr? */
    {
        /* thread already destroyed or not created */
        return;
    }

    /* !!!! _exitthreadex does _not_ call CloseHandle !!! */
    CloseHandle( pThreadImpl->m_hThread );

    /* free memory */
    free(Thread);
}

/*****************************************************************************/
/* osl_resumeThread */
/*****************************************************************************/
void SAL_CALL osl_resumeThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    ResumeThread(pThreadImpl->m_hThread);
}

/*****************************************************************************/
/* osl_suspendThread */
/*****************************************************************************/
void SAL_CALL osl_suspendThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    SuspendThread(pThreadImpl->m_hThread);
}

/*****************************************************************************/
/* osl_setThreadPriority */
/*****************************************************************************/
void SAL_CALL osl_setThreadPriority(oslThread Thread,
                           oslThreadPriority Priority)
{
    int winPriority;
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

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

/*****************************************************************************/
/* osl_getThreadPriority  */
/*****************************************************************************/
oslThreadPriority SAL_CALL osl_getThreadPriority(const oslThread Thread)
{
    int winPriority;
    oslThreadPriority Priority;

    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments ?*/
    if(pThreadImpl==0 || pThreadImpl->m_hThread==0)
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
        OSL_ASSERT(FALSE);      /* WIN32 API changed, incorporate new prio-level! */

        /* release-version behaves friendly */
        Priority= osl_Thread_PriorityUnknown;
    }

    return Priority;
}

/*****************************************************************************/
/* osl_isThreadRunning */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isThreadRunning(const oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments ?*/
    if(pThreadImpl==0 || pThreadImpl->m_hThread==0)
    {
        return sal_False;
    }

    return (sal_Bool)(WaitForSingleObject(pThreadImpl->m_hThread, 0) != WAIT_OBJECT_0);
}

/*****************************************************************************/
/* osl_joinWithThread */
/*****************************************************************************/
void SAL_CALL osl_joinWithThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments?*/
    if(pThreadImpl==0 || pThreadImpl->m_hThread==0)
    {
        /* assume thread is not running */
        return;
    }

    WaitForSingleObject(pThreadImpl->m_hThread, INFINITE);
}

/*****************************************************************************/
/* osl_waitThread */
/*****************************************************************************/
void SAL_CALL osl_waitThread(const TimeValue* pDelay)
{
    if (pDelay)
    {
        DWORD millisecs = pDelay->Seconds * 1000L + pDelay->Nanosec / 1000000L;

        Sleep(millisecs);
    }
}

/*****************************************************************************/
/* osl_terminateThread */
/*****************************************************************************/
void SAL_CALL osl_terminateThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments?*/
    if (pThreadImpl==0 || pThreadImpl->m_hThread==0)
    {
        /* assume thread is not running */
        return;
    }

    osl_atomic_increment(&(pThreadImpl->m_nTerminationRequested));
}


/*****************************************************************************/
/* osl_scheduleThread */
/*****************************************************************************/
sal_Bool SAL_CALL osl_scheduleThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    osl_yieldThread();

    /* invalid arguments?*/
    if (pThreadImpl==0 || pThreadImpl->m_hThread==0)
    {
        /* assume thread is not running */
        return sal_False;
    }

    return (sal_Bool)(0 == pThreadImpl->m_nTerminationRequested);
}

/*****************************************************************************/
/* osl_yieldThread */
/*****************************************************************************/
void SAL_CALL osl_yieldThread(void)
{
    Sleep(0);
}

void SAL_CALL osl_setThreadName(char const * name) {
#ifdef _MSC_VER
    /* See <http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx>: */
#pragma pack(push, 8)
    struct {
        DWORD dwType;
        LPCSTR szName;
        DWORD dwThreadID;
        DWORD dwFlags;
    } info;
#pragma pack(pop)
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = (DWORD) -1;
    info.dwFlags = 0;
    __try {
        RaiseException(
            0x406D1388, 0, sizeof info / sizeof (ULONG_PTR),
            (ULONG_PTR *) &info);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
#else
    (void) name;
#endif
}

typedef struct _TLS
{
    DWORD                           dwIndex;
    oslThreadKeyCallbackFunction    pfnCallback;
    struct _TLS                     *pNext, *pPrev;
} TLS, *PTLS;

static  PTLS        g_pThreadKeyList = NULL;
CRITICAL_SECTION    g_ThreadKeyListCS;

static void AddKeyToList( PTLS pTls )
{
    if ( pTls )
    {
        EnterCriticalSection( &g_ThreadKeyListCS );

        pTls->pNext = g_pThreadKeyList;
        pTls->pPrev = 0;

        if ( g_pThreadKeyList )
            g_pThreadKeyList->pPrev = pTls;

        g_pThreadKeyList = pTls;

        LeaveCriticalSection( &g_ThreadKeyListCS );
    }
}

static void RemoveKeyFromList( PTLS pTls )
{
    if ( pTls )
    {
        EnterCriticalSection( &g_ThreadKeyListCS );
        if ( pTls->pPrev )
            pTls->pPrev->pNext = pTls->pNext;
        else
        {
            OSL_ASSERT( pTls == g_pThreadKeyList );
            g_pThreadKeyList = pTls->pNext;
        }

        if ( pTls->pNext )
            pTls->pNext->pPrev = pTls->pPrev;
        LeaveCriticalSection( &g_ThreadKeyListCS );
    }
}

void SAL_CALL _osl_callThreadKeyCallbackOnThreadDetach(void)
{
    PTLS    pTls;


    EnterCriticalSection( &g_ThreadKeyListCS );
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
    LeaveCriticalSection( &g_ThreadKeyListCS );
}

/*****************************************************************************/
/* osl_createThreadKey */
/*****************************************************************************/
oslThreadKey SAL_CALL osl_createThreadKey(oslThreadKeyCallbackFunction pCallback)
{
    PTLS    pTls = rtl_allocateMemory( sizeof(TLS) );

    if ( pTls )
    {
        pTls->pfnCallback = pCallback;
        if ( (DWORD)-1 == (pTls->dwIndex = TlsAlloc()) )
        {
            rtl_freeMemory( pTls );
            pTls = 0;
        }
        else
            AddKeyToList( pTls );
    }

    return ((oslThreadKey)pTls);
}

/*****************************************************************************/
/* osl_destroyThreadKey */
/*****************************************************************************/
void SAL_CALL osl_destroyThreadKey(oslThreadKey Key)
{
    if (Key != 0)
    {
        PTLS    pTls = (PTLS)Key;

        RemoveKeyFromList( pTls );
        TlsFree( pTls->dwIndex );
        rtl_freeMemory( pTls );
    }
}

/*****************************************************************************/
/* osl_getThreadKeyData */
/*****************************************************************************/
void* SAL_CALL osl_getThreadKeyData(oslThreadKey Key)
{
    if (Key != 0)
    {
        PTLS    pTls = (PTLS)Key;

        return (TlsGetValue( pTls->dwIndex ));
    }

    return (NULL);
}

/*****************************************************************************/
/* osl_setThreadKeyData */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setThreadKeyData(oslThreadKey Key, void *pData)
{
    if (Key != 0)
    {
        PTLS    pTls = (PTLS)Key;
        void*   pOldData = NULL;
        BOOL    fSuccess;

        if ( pTls->pfnCallback )
            pOldData = TlsGetValue( pTls->dwIndex );

        fSuccess = TlsSetValue( pTls->dwIndex, pData );

        if ( fSuccess && pTls->pfnCallback && pOldData )
            pTls->pfnCallback( pOldData );

        return (sal_Bool)(fSuccess != FALSE);
    }

    return (sal_False);
}


/*****************************************************************************/
/* osl_getThreadTextEncoding */
/*****************************************************************************/

DWORD   g_dwTLSTextEncodingIndex = (DWORD)-1;


rtl_TextEncoding SAL_CALL osl_getThreadTextEncoding(void)
{
    DWORD               dwEncoding;
    rtl_TextEncoding    _encoding;
    BOOL                gotACP;

    if ( (DWORD)-1 == g_dwTLSTextEncodingIndex )
        g_dwTLSTextEncodingIndex = TlsAlloc();

    dwEncoding = (DWORD)TlsGetValue( g_dwTLSTextEncodingIndex );
    _encoding = LOWORD(dwEncoding);
    gotACP = HIWORD(dwEncoding);


    if ( !gotACP )
    {
        char    *pszEncoding;

        if ( NULL != (pszEncoding = getenv( "SOLAR_USER_RTL_TEXTENCODING" )) )
            _encoding = (rtl_TextEncoding)atoi(pszEncoding);
        else
            _encoding = rtl_getTextEncodingFromWindowsCodePage( GetACP() );

        TlsSetValue( g_dwTLSTextEncodingIndex, (LPVOID)MAKELONG( _encoding, TRUE ) );
    }

    return _encoding;
}

/*****************************************************************************/
/* osl_getThreadTextEncoding */
/*****************************************************************************/
rtl_TextEncoding SAL_CALL osl_setThreadTextEncoding( rtl_TextEncoding Encoding )
{
    rtl_TextEncoding oldEncoding = osl_getThreadTextEncoding();

    TlsSetValue( g_dwTLSTextEncodingIndex, (LPVOID)MAKELONG( Encoding, TRUE) );

    return oldEncoding;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
