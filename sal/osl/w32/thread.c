/*************************************************************************
 *
 *  $RCSfile: thread.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: martin.maher $ $Date: 2000-09-29 14:37:19 $
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

#include "system.h"

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/alloc.h>

/*
    Thread-data structure hidden behind oslThread:
*/
typedef struct _osl_TThreadImpl
{
    HANDLE              m_hThread;      /* OS-handle used for all thread-functions */
    sal_uInt32          m_ThreadId;     /* identifier for this thread */
    sal_uInt32          m_Flags;
    HANDLE              m_hEvent;
    DWORD               m_Timeout;
    CRITICAL_SECTION    m_Mutex;
    oslWorkerFunction   m_WorkerFunction;
    void*               m_pData;

} osl_TThreadImpl;

#define THREADIMPL_FLAGS_TERMINATE  0x0001
#define THREADIMPL_FLAGS_SLEEP      0x0002

static sal_uInt32 __stdcall oslWorkerWrapperFunction(void* pData);
static oslThread oslCreateThread(oslWorkerFunction pWorker, void* pThreadData, sal_uInt32 nFlags);
static HRESULT WINAPI osl_CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);

typedef HRESULT (WINAPI *CoInitializeEx_PROC)(LPVOID pvReserved, DWORD dwCoInit);

CoInitializeEx_PROC _CoInitializeEx = osl_CoInitializeEx;

/*****************************************************************************/
/* oslWorkerWrapperFunction */
/*****************************************************************************/
static sal_uInt32 __stdcall oslWorkerWrapperFunction(void* pData)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)pData;

    /* Initialize COM */

    _CoInitializeEx(NULL, COINIT_MULTITHREADED);

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

    pThreadImpl->m_Flags   = 0;
    pThreadImpl->m_hEvent  = 0;
    pThreadImpl->m_Timeout = 0;

    InitializeCriticalSection(&pThreadImpl->m_Mutex);

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
        if (pThreadImpl->m_hEvent != 0)
            CloseHandle(pThreadImpl->m_hEvent);

        DeleteCriticalSection(&pThreadImpl->m_Mutex);

        free(pThreadImpl);
        return 0;
    }

    return (oslThread)pThreadImpl;
}

/*****************************************************************************/
/* osl_ CoInitializeEx */
/*****************************************************************************/

static HRESULT WINAPI osl_CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit)
{
#ifdef _WIN32_DCOM // DCOM
    return CoInitializeEx( pvReserved, dwCoInit );
#else
    return CoInitialize( pvReserved );
#endif
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

    if (pThreadImpl->m_hThread != 0)    /* valid handle ? */
    {
        /* cancel thread  */
        TerminateThread(pThreadImpl->m_hThread, 0);
    }
}

/*****************************************************************************/
/* osl_freeThreadHandle */
/*****************************************************************************/
void SAL_CALL osl_freeThreadHandle(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    if(Thread == 0)     /* valid ptr? */
    {
        /* thread already destroyed or not created */
        return;
    }

    if (pThreadImpl->m_hEvent != 0)
        CloseHandle(pThreadImpl->m_hEvent);

    DeleteCriticalSection(&pThreadImpl->m_Mutex);

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

    return (WaitForSingleObject(pThreadImpl->m_hThread, 0) != WAIT_OBJECT_0);
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
/* osl_sleepThread */
/*****************************************************************************/
oslThreadSleep SAL_CALL osl_sleepThread(oslThread Thread, const TimeValue* pDelay)
{
    DWORD            millisecs;
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments?*/
    if (pThreadImpl==0 || pThreadImpl->m_hThread==0 || pDelay==0)
        return osl_Thread_SleepError;

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SLEEP)
        return osl_Thread_SleepActive;

    EnterCriticalSection(&pThreadImpl->m_Mutex);

    millisecs = pDelay->Seconds * 1000L + pDelay->Nanosec / 1000000L;

    if (pThreadImpl->m_hEvent == 0)
        pThreadImpl->m_hEvent= CreateEvent(NULL, FALSE, FALSE, NULL);
    else
        ResetEvent(pThreadImpl->m_hEvent);

    if (pThreadImpl->m_ThreadId == GetCurrentThreadId())
    {
        DWORD ret;

        pThreadImpl->m_Timeout = 0;
        pThreadImpl->m_Flags |=  THREADIMPL_FLAGS_SLEEP;

        LeaveCriticalSection(&pThreadImpl->m_Mutex);

        ret = WaitForSingleObject(pThreadImpl->m_hEvent, millisecs);

        EnterCriticalSection(&pThreadImpl->m_Mutex);

        pThreadImpl->m_Flags &=  ~THREADIMPL_FLAGS_SLEEP;

        LeaveCriticalSection(&pThreadImpl->m_Mutex);

        return (ret == WAIT_TIMEOUT) ? osl_Thread_SleepNormal :
                                       osl_Thread_SleepCancel;
    }
    else
    {
        pThreadImpl->m_Timeout = millisecs;
        pThreadImpl->m_Flags |=  THREADIMPL_FLAGS_SLEEP;

        LeaveCriticalSection(&pThreadImpl->m_Mutex);

        return osl_Thread_SleepPending;
    }
}

/*****************************************************************************/
/* osl_awakeThread */
/*****************************************************************************/
sal_Bool SAL_CALL osl_awakeThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments?*/
    if (pThreadImpl==0 || pThreadImpl->m_hThread==0)
    {
        /* assume thread is not running */
        return sal_False;
    }

    EnterCriticalSection(&pThreadImpl->m_Mutex);

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SLEEP)
    {
        SetEvent(pThreadImpl->m_hEvent);
    }

    pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_SLEEP;

    LeaveCriticalSection(&pThreadImpl->m_Mutex);

    return sal_True;
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

    EnterCriticalSection(&pThreadImpl->m_Mutex);
    pThreadImpl->m_Flags |= THREADIMPL_FLAGS_TERMINATE;
    LeaveCriticalSection(&pThreadImpl->m_Mutex);
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

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SLEEP)
    {
        OSL_ASSERT (pThreadImpl->m_hEvent != 0);

        WaitForSingleObject(pThreadImpl->m_hEvent, pThreadImpl->m_Timeout);

        EnterCriticalSection(&pThreadImpl->m_Mutex);

        pThreadImpl->m_Timeout = 0;

        pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_SLEEP;

        LeaveCriticalSection(&pThreadImpl->m_Mutex);
    }

    return ((pThreadImpl->m_Flags & THREADIMPL_FLAGS_TERMINATE) == 0);
}

/*****************************************************************************/
/* osl_yieldThread */
/*****************************************************************************/
void SAL_CALL osl_yieldThread()
{
    Sleep(0);
}

typedef struct _TLS
{
    DWORD                           dwIndex;
    oslThreadKeyCallbackFunction    pfnCallback;
    struct _TLS                     *pNext, *pPrev;
} TLS, *PTLS;

static  PTLS    g_pThreadKeyList = NULL;

static void AddKeyToList( PTLS pTls )
{
    if ( pTls )
    {
        pTls->pNext = g_pThreadKeyList;
        pTls->pPrev = 0;

        if ( g_pThreadKeyList )
            g_pThreadKeyList->pPrev = pTls;

        g_pThreadKeyList = pTls;
    }
}

static void RemoveKeyFromList( PTLS pTls )
{
    if ( pTls )
    {
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

void SAL_CALL _osl_callThreadKeyCallbackOnThreadDetach()
{
    PTLS    pTls = g_pThreadKeyList;

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
        void*   pOldData;
        BOOL    fSuccess;

        if ( pTls->pfnCallback )
            pOldData = TlsGetValue( pTls->dwIndex );

        fSuccess = TlsSetValue( pTls->dwIndex, pData );

        if ( fSuccess && pTls->pfnCallback && pOldData )
            pTls->pfnCallback( pOldData );

        return (fSuccess != FALSE);
    }

    return (sal_False);
}


/*****************************************************************************/
/* osl_getThreadTextEncoding */
/*****************************************************************************/

DWORD   g_dwTLSTextEncodingIndex = (DWORD)-1;


rtl_TextEncoding SAL_CALL osl_getThreadTextEncoding()
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
        switch ( GetACP() )
        {
            case 1200:
                _encoding = RTL_TEXTENCODING_UNICODE;
                break;
            case 1252:
                _encoding = RTL_TEXTENCODING_MS_1252;
                break;
            case 1250:
                _encoding = RTL_TEXTENCODING_MS_1250;
                break;
            case 1251:
                _encoding = RTL_TEXTENCODING_MS_1251;
                break;
            case 1253:
                _encoding = RTL_TEXTENCODING_MS_1253;
                break;
            case 1254:
                _encoding = RTL_TEXTENCODING_MS_1254;
                break;
            case 1255:
                _encoding = RTL_TEXTENCODING_MS_1255;
                break;
            case 1256:
                _encoding = RTL_TEXTENCODING_MS_1256;
                break;
            case 1257:
                _encoding = RTL_TEXTENCODING_MS_1257;
                break;
            case 1258:
                _encoding = RTL_TEXTENCODING_MS_1258;
                break;
            case 874:
                _encoding = RTL_TEXTENCODING_MS_874;
                break;
            case 932:
                _encoding = RTL_TEXTENCODING_MS_932;
                break;
            case 936:
                _encoding = RTL_TEXTENCODING_MS_936;
                break;
            case 949:
                _encoding = RTL_TEXTENCODING_MS_949;
                break;
            case 950:
                _encoding = RTL_TEXTENCODING_MS_950;
                break;
            default:
                _encoding = RTL_TEXTENCODING_DONTKNOW;
                break;
        }

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



