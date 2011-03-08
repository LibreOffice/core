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


#include "system.h"

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/time.h>
#include <rtl/alloc.h>
#include <rtl/tencinfo.h>

/*
    Thread-data structure hidden behind oslThread:
*/
typedef struct _osl_TThreadImpl
{

    TID                  m_ThreadId;        /* identifier for this thread */
    sal_Int32            m_Flags;
    HEV                  m_hEvent;
    sal_uInt32           m_Timeout;
    oslWorkerFunction    m_WorkerFunction;
    void*                m_pData;
    sal_Bool             m_StartSuspended;
    HAB                  m_hab;
    HMQ                  m_hmq;

} osl_TThreadImpl;

#define THREADIMPL_FLAGS_TERMINATE    0x0001
#define THREADIMPL_FLAGS_SLEEP        0x0002


// static mutex to control access to private members of oslMutexImpl
static HMTX MutexLock = NULL;

/*****************************************************************************/

HAB osl_getPMinternal_HAB(oslThread hThread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)hThread;

    if(pThreadImpl == NULL) /* valid ptr? */
    {
        return NULL;
    }
    else
    {
        return pThreadImpl->m_hab;
    }
}

HMQ osl_getPMinternal_HMQ(oslThread hThread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)hThread;

    if(pThreadImpl == NULL) /* valid ptr? */
    {
        return NULL;
    }
    else
    {
        return pThreadImpl->m_hmq;
    }
}


/*****************************************************************************/
/* oslWorkerWrapperFunction */
/*****************************************************************************/
static void oslWorkerWrapperFunction(void* pData)
{
    BOOL rc;
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)pData;

#if OSL_DEBUG_LEVEL > 0
printf("oslWorkerWrapperFunction pThreadImpl %x, pThreadImpl->m_ThreadId %d\n", pThreadImpl, pThreadImpl->m_ThreadId);
#endif
    /* Inizialize PM for this thread */
    pThreadImpl->m_hab = WinInitialize( 0 );
#if OSL_DEBUG_LEVEL > 0
printf("pThreadImpl->m_ThreadId %d, pThreadImpl->m_hab %x\n", pThreadImpl->m_ThreadId,pThreadImpl->m_hab);
#endif
    pThreadImpl->m_hmq = WinCreateMsgQueue( pThreadImpl->m_hab, 0 );
#if OSL_DEBUG_LEVEL > 0
printf("pThreadImpl->m_ThreadId %d, pThreadImpl->m_hmq %x\n", pThreadImpl->m_ThreadId,pThreadImpl->m_hmq);
#endif

    /* call worker-function with data */
    pThreadImpl->m_WorkerFunction( pThreadImpl->m_pData );

    /* Free all PM-resources for this thread */
#if OSL_DEBUG_LEVEL > 0
printf("pThreadImpl->m_ThreadId %d, about to destroy queue\n", pThreadImpl->m_ThreadId);
#endif
    rc = WinDestroyMsgQueue( pThreadImpl->m_hmq );
#if OSL_DEBUG_LEVEL > 0
printf("pThreadImpl->m_ThreadId %d, WinDestroyMsgQueue rc=%d (should be 1)\n", pThreadImpl->m_ThreadId, rc);
printf("pThreadImpl->m_ThreadId %d, about to terminate hab\n", pThreadImpl->m_ThreadId);
#endif
    rc = WinTerminate( pThreadImpl->m_hab );
#if OSL_DEBUG_LEVEL > 0
printf("pThreadImpl->m_ThreadId %d, WinTerminate rc=%d (should be 1)\n", pThreadImpl->m_ThreadId, rc);
#endif
}


/*****************************************************************************/
/* oslCreateThread */
/*****************************************************************************/
static oslThread oslCreateThread(oslWorkerFunction pWorker,
                                 void* pThreadData,
                                 sal_Bool nFlags)
{
    osl_TThreadImpl* pThreadImpl;

    /* alloc mem. for our internal data structure */
    pThreadImpl = (osl_TThreadImpl*)malloc(sizeof(osl_TThreadImpl));

    OSL_ASSERT(pThreadImpl);

    pThreadImpl->m_WorkerFunction= pWorker;
    pThreadImpl->m_pData= pThreadData;

    pThreadImpl->m_Flags   = 0;
    pThreadImpl->m_hEvent  = 0;
    pThreadImpl->m_Timeout = 0;
    pThreadImpl->m_StartSuspended = nFlags;
    pThreadImpl->m_hab = 0;
    pThreadImpl->m_hmq = 0;

    if ( nFlags == sal_True )
    {
        DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );
    }

    pThreadImpl->m_ThreadId = (TID) _beginthread( oslWorkerWrapperFunction,    /* worker-function */
                                                  NULL,                        /* unused parameter */
                                                  1024*1024,                   /* max. Stacksize */
                                                  pThreadImpl );
    if ( nFlags == sal_True )
    {
        if( pThreadImpl->m_ThreadId != -1 )
            DosSuspendThread( pThreadImpl->m_ThreadId );
        DosReleaseMutexSem( MutexLock);
    }
#if OSL_DEBUG_LEVEL > 0
printf("oslCreateThread pThreadImpl %x, pThreadImpl->m_ThreadId %d\n", pThreadImpl, pThreadImpl->m_ThreadId);
#endif
    if(pThreadImpl->m_ThreadId == -1)
    {
        /* create failed */
        if (pThreadImpl->m_hEvent != 0)
            DosCloseEventSem(pThreadImpl->m_hEvent);

        free(pThreadImpl);
        return 0;
    }

    pThreadImpl->m_hEvent= 0;

    return pThreadImpl;

}

/*****************************************************************************/
/* osl_createThread */
/*****************************************************************************/
oslThread SAL_CALL osl_createThread(oslWorkerFunction pWorker,
                                 void* pThreadData)
{
    return oslCreateThread(pWorker,pThreadData,sal_False);
}

/*****************************************************************************/
/* osl_createSuspendedThread */
/*****************************************************************************/
oslThread SAL_CALL osl_createSuspendedThread(oslWorkerFunction pWorker,
                                          void* pThreadData)
{
    return oslCreateThread(pWorker,pThreadData,sal_True);
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
        {
        PTIB pptib = NULL;
        PPIB pppib = NULL;

        DosGetInfoBlocks( &pptib, &pppib );
        return ((oslThreadIdentifier) pptib->tib_ptib2->tib2_ultid );
        }
}

/*****************************************************************************/
/* osl_destroyThread */
/*****************************************************************************/
void SAL_CALL osl_destroyThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    if(Thread == 0) /* valid ptr? */
    {
        /* thread already destroyed or not created */
        return;
    }

    if(pThreadImpl->m_ThreadId != -1)    /* valid handle ? */
    {
        /* cancel thread  */
        DosKillThread( pThreadImpl->m_ThreadId );
    }
}

/*****************************************************************************/
/* osl_freeThreadHandle */
/*****************************************************************************/
void SAL_CALL osl_freeThreadHandle(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    if(Thread == 0)        /* valid ptr? */
    {
        /* thread already destroyed or not created */
        return;
    }

    if (pThreadImpl->m_hEvent != 0)
        DosCloseEventSem(pThreadImpl->m_hEvent);

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

    DosResumeThread( pThreadImpl->m_ThreadId );
}

/*****************************************************************************/
/* osl_suspendThread */
/*****************************************************************************/
void SAL_CALL osl_suspendThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    DosSuspendThread( pThreadImpl->m_ThreadId );
}

/*****************************************************************************/
/* osl_setThreadPriority */
/*****************************************************************************/
void SAL_CALL osl_setThreadPriority(oslThread Thread,
                           oslThreadPriority Priority)
{
    ULONG nOs2PriorityClass;
    ULONG nOs2PriorityDelta;
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    OSL_ASSERT(pThreadImpl);        /* valid ptr? */

    switch(Priority) {

    case osl_Thread_PriorityHighest:

        nOs2PriorityClass = PRTYC_REGULAR;
        nOs2PriorityDelta = PRTYD_MAXIMUM;
        break;

    case osl_Thread_PriorityAboveNormal:

        nOs2PriorityClass = PRTYC_REGULAR;
        nOs2PriorityDelta = 16;
        break;

    case osl_Thread_PriorityNormal:

        nOs2PriorityClass = PRTYC_REGULAR;
        nOs2PriorityDelta = 0;
        break;

    case osl_Thread_PriorityBelowNormal:

        nOs2PriorityClass = PRTYC_REGULAR;
        nOs2PriorityDelta = -16;
        break;

    case osl_Thread_PriorityLowest:

        nOs2PriorityClass = PRTYC_REGULAR;
        nOs2PriorityDelta = PRTYD_MINIMUM;
        break;

    case osl_Thread_PriorityUnknown:
        OSL_ASSERT(FALSE);        /* only fools try this...*/

        /* let release-version behave friendly */
        return;

    default:
        OSL_ASSERT(FALSE);        /* enum expanded, but forgotten here...*/

        /* let release-version behave friendly */
        return;
    }

    DosSetPriority( PRTYS_THREAD,
                    nOs2PriorityClass, nOs2PriorityDelta,
                    pThreadImpl->m_ThreadId );

}

/*****************************************************************************/
/* osl_getThreadPriority  */
/*****************************************************************************/

#define BYTE1FROMULONG(ul) ((UCHAR) (ul))
#define BYTE2FROMULONG(ul) ((UCHAR) ((ULONG) ul >> 8))

oslThreadPriority  SAL_CALL osl_getThreadPriority(const oslThread Thread)
{
    ULONG nOs2PriorityClass;
    ULONG nOs2PriorityDelta;

    oslThreadPriority Priority;

    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments ?*/
    if(pThreadImpl==0 || pThreadImpl->m_ThreadId==-1)
    {
        return osl_Thread_PriorityUnknown;
    }

    /* get current priorities */
    {
    PTIB pptib = NULL;
    PPIB pppib = NULL;

    DosGetInfoBlocks( &pptib, &pppib );
    nOs2PriorityClass = BYTE1FROMULONG( pptib->tib_ptib2->tib2_ulpri );
    nOs2PriorityDelta = BYTE2FROMULONG( pptib->tib_ptib2->tib2_ulpri );
    }

    /* map OS2 priority to enum */
    switch(nOs2PriorityClass)
    {
    case PRTYC_TIMECRITICAL:
        Priority= osl_Thread_PriorityHighest;
        break;

    case PRTYC_REGULAR:

        if( nOs2PriorityDelta == 0 )
        {
            Priority= osl_Thread_PriorityNormal;
            break;
        }

        if( nOs2PriorityDelta < -16 )
        {
            Priority= osl_Thread_PriorityLowest;
            break;
        }

        if( nOs2PriorityDelta < 0 )
        {
            Priority= osl_Thread_PriorityBelowNormal;
            break;
        }

        if( nOs2PriorityDelta > 0 )
        {
            Priority= osl_Thread_PriorityAboveNormal;
            break;
        }

        Priority= osl_Thread_PriorityHighest;
        break;

    case PRTYC_IDLETIME:
        Priority= osl_Thread_PriorityLowest;
        break;

    default:
        OSL_ASSERT(FALSE);        /* OS/2 API changed, incorporate new prio-level! */

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
    APIRET rc;

    /* invalid arguments ?*/
    if(pThreadImpl==0 || pThreadImpl->m_ThreadId==-1)
    {
        return sal_False;
    }

    if( osl_getThreadIdentifier( 0 ) == osl_getThreadIdentifier( Thread ) )
        return sal_True;

    rc = DosWaitThread( &pThreadImpl->m_ThreadId, DCWW_NOWAIT );

    return( rc != ERROR_INVALID_THREADID );
}

/*****************************************************************************/
/* osl_joinWithThread */
/*****************************************************************************/
void SAL_CALL osl_joinWithThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments?*/
    if(pThreadImpl==0 || pThreadImpl->m_ThreadId==-1)
    {
        /* assume thread is not running */
        return;
    }

    DosWaitThread( &pThreadImpl->m_ThreadId, DCWW_WAIT );
}

/*****************************************************************************/
/* osl_waitThread */
/*****************************************************************************/
void SAL_CALL osl_waitThread(const TimeValue* pDelay)
{
    int millisecs;

    OSL_ASSERT(pDelay);

    millisecs = pDelay->Seconds * 1000 + pDelay->Nanosec / 1000000;

    DosSleep(millisecs);
}

/*****************************************************************************/
/* osl_terminateThread */
/*****************************************************************************/
void SAL_CALL osl_terminateThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    /* invalid arguments?*/
    if (pThreadImpl==0 || pThreadImpl->m_ThreadId==-1)
    {
        /* assume thread is not running */
        return;
    }

    DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );
    pThreadImpl->m_Flags |= THREADIMPL_FLAGS_TERMINATE;
    DosReleaseMutexSem( MutexLock);
}


/*****************************************************************************/
/* osl_scheduleThread */
/*****************************************************************************/
sal_Bool SAL_CALL osl_scheduleThread(oslThread Thread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    osl_yieldThread();

    /* invalid arguments?*/
    if (pThreadImpl==0 || pThreadImpl->m_ThreadId==-1)
    {
        /* assume thread is not running */
        return sal_False;
    }

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SLEEP)
    {
        OSL_ASSERT (pThreadImpl->m_hEvent != 0);

        DosWaitEventSem(pThreadImpl->m_hEvent, pThreadImpl->m_Timeout);

        DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );

        pThreadImpl->m_Timeout = 0;

        pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_SLEEP;

        DosReleaseMutexSem( MutexLock);
    }

    return ((pThreadImpl->m_Flags & THREADIMPL_FLAGS_TERMINATE) == 0);
}

/*****************************************************************************/
/* osl_yieldThread */
/*****************************************************************************/
void SAL_CALL osl_yieldThread()
{
    DosSleep(0);
}

typedef struct _TLS
{
    PULONG                          pulPtr;
    oslThreadKeyCallbackFunction    pfnCallback;
    struct _TLS                     *pNext, *pPrev;
} TLS, *PTLS;

static  PTLS        g_pThreadKeyList = NULL;

static void AddKeyToList( PTLS pTls )
{
    if ( pTls )
    {
        DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );

        pTls->pNext = g_pThreadKeyList;
        pTls->pPrev = 0;

        if ( g_pThreadKeyList )
            g_pThreadKeyList->pPrev = pTls;

        g_pThreadKeyList = pTls;

        DosReleaseMutexSem( MutexLock);
    }
}

static void RemoveKeyFromList( PTLS pTls )
{
    if ( pTls )
    {
        DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );
        if ( pTls->pPrev )
            pTls->pPrev->pNext = pTls->pNext;
        else
        {
            OSL_ASSERT( pTls == g_pThreadKeyList );
            g_pThreadKeyList = pTls->pNext;
        }

        if ( pTls->pNext )
            pTls->pNext->pPrev = pTls->pPrev;
        DosReleaseMutexSem( MutexLock);
    }
}

void SAL_CALL _osl_callThreadKeyCallbackOnThreadDetach(void)
{
    PTLS    pTls;

    DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );
    pTls = g_pThreadKeyList;
    while ( pTls )
    {
        if ( pTls->pfnCallback )
        {
            void    *pValue = (void*)*pTls->pulPtr;

            if ( pValue )
                pTls->pfnCallback( pValue );
        }

        pTls = pTls->pNext;
    }
    DosReleaseMutexSem( MutexLock);
}

/*****************************************************************************/
/* osl_createThreadKey */
/*****************************************************************************/
oslThreadKey SAL_CALL osl_createThreadKey(oslThreadKeyCallbackFunction pCallback)
{
    PTLS    pTls = (PTLS)rtl_allocateMemory( sizeof(TLS) );

    if ( pTls )
    {
        pTls->pfnCallback = pCallback;
        if (DosAllocThreadLocalMemory(1, &pTls->pulPtr) != NO_ERROR)
        {
            rtl_freeMemory( pTls );
            pTls = 0;
        }
        else
        {
            *pTls->pulPtr = 0;
            AddKeyToList( pTls );
        }
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
        DosFreeThreadLocalMemory(pTls->pulPtr);
        rtl_freeMemory( pTls );
    }
}

/*****************************************************************************/
/* osl_getThreadKeyData */
/*****************************************************************************/
void * SAL_CALL osl_getThreadKeyData(oslThreadKey Key)
{
    if (Key != 0)
    {
        PTLS    pTls = (PTLS)Key;

        return ((void *) *pTls->pulPtr);
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
        BOOL    fSuccess = TRUE; //YD cannot fail

        if ( pTls->pfnCallback )
            pOldData = (void*)*pTls->pulPtr;

        *pTls->pulPtr = (ULONG)pData;

        if ( fSuccess && pTls->pfnCallback && pOldData )
            pTls->pfnCallback( pOldData );

        return (sal_Bool)(fSuccess != FALSE);
    }

    return (sal_False);
}



/*****************************************************************************/
/* osl_getThreadTextEncoding */
/*****************************************************************************/

ULONG   g_dwTLSTextEncodingIndex = (ULONG)-1;

sal_uInt32 SAL_CALL _GetACP( void)
{
    APIRET  rc;
    ULONG   aulCpList[8]  = {0};
    ULONG   ulListSize;

    rc = DosQueryCp( sizeof( aulCpList), aulCpList, &ulListSize);
    if (rc)
        return 437; // in case of error, return codepage EN_US
    // current codepage is first of list, others are the prepared codepages.
    return aulCpList[0];
}

rtl_TextEncoding SAL_CALL osl_getThreadTextEncoding(void)
{
    rtl_TextEncoding    _encoding;

    if ( (ULONG)-1 == g_dwTLSTextEncodingIndex ) {
        rtl_TextEncoding defaultEncoding;
        const char *     pszEncoding;

        /* create thread specific data key */
        g_dwTLSTextEncodingIndex = osl_createThreadKey( NULL);

        /* determine default text encoding */
        pszEncoding = getenv ("SOLAR_USER_RTL_TEXTENCODING");
        if (pszEncoding)
            defaultEncoding = atoi(pszEncoding);
        else
            defaultEncoding = rtl_getTextEncodingFromWindowsCodePage( _GetACP());

        //OSL_ASSERT(defaultEncoding != RTL_TEXTENCODING_DONTKNOW);
        //g_thread.m_textencoding.m_default = defaultEncoding;
        osl_setThreadKeyData( g_dwTLSTextEncodingIndex, (void*)defaultEncoding);
    }

    _encoding = (rtl_TextEncoding)osl_getThreadKeyData( g_dwTLSTextEncodingIndex );
    if (0 == _encoding) {
        const char *     pszEncoding;
        /* determine default text encoding */
        pszEncoding = getenv ("SOLAR_USER_RTL_TEXTENCODING");
        if (pszEncoding)
            _encoding = atoi(pszEncoding);
        else
            _encoding = rtl_getTextEncodingFromWindowsCodePage( _GetACP());
        /* save for future reference */
        osl_setThreadKeyData( g_dwTLSTextEncodingIndex, (void*)_encoding);
    }

    return _encoding;
}

/*****************************************************************************/
/* osl_getThreadTextEncoding */
/*****************************************************************************/
rtl_TextEncoding SAL_CALL osl_setThreadTextEncoding( rtl_TextEncoding Encoding )
{
    rtl_TextEncoding oldEncoding = osl_getThreadTextEncoding();

    osl_setThreadKeyData( g_dwTLSTextEncodingIndex, (void*)Encoding);

    return oldEncoding;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
