/*************************************************************************
 *
 *  $RCSfile: thread.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 09:48:10 $
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
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

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

HAB osl_getPMinternal_HAB(oslThread hThread)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)hThread;

    if(pThreadImpl == NULL) /* valid ptr? */
    {
        return NULL;
    }
    else
    {o
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
static void _Optlink oslWorkerWrapperFunction(void* pData)
{
    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)pData;

    /* Inizialize PM for this thread */
    pThreadImpl->m_hab = WinInitialize( 0 );
    pThreadImpl->m_hmq = WinCreateMsgQueue( pThreadImpl->m_hab, 0 );

    /* call worker-function with data */
    pThreadImpl->m_WorkerFunction( pThreadImpl->m_pData );

    /* Free all PM-resources for this thread */
    WinDestroyMsgQueue( pThreadImpl->m_hmq );
    WinTerminate( pThreadImpl->m_hab );
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
    pThreadImpl = malloc(sizeof(osl_TThreadImpl));

    OSL_ASSERT(pThreadImpl);

    pThreadImpl->m_WorkerFunction= pWorker;
    pThreadImpl->m_pData= pThreadData;

    pThreadImpl->m_Flags   = 0;
    pThreadImpl->m_hEvent  = 0;
    pThreadImpl->m_Timeout = 0;
    pThreadImpl->m_StartSuspended = nFlags;

    if ( nFlags == sal_True )
    {
        DosEnterCritSec();
    }

    pThreadImpl->m_ThreadId = (TID) _beginthread( oslWorkerWrapperFunction,    /* worker-function */
                                                  NULL,                        /* unused parameter */
                                                  1024000,                     /* max. Stacksize */
                                                  pThreadImpl );
    if ( nFlags == sal_True )
    {
        if( pThreadImpl->m_ThreadId != -1 )
            DosSuspendThread( pThreadImpl->m_ThreadId );
        DosExitCritSec();
    }

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
/* osl_sleepThread */
/*****************************************************************************/
oslThreadSleep SAL_CALL osl_sleepThread(oslThread Thread, const TimeValue* pDelay)
{
    TID tidCurrentThread;
    APIRET rc;
    ULONG ulPostCount;
    int   millisecs;

    osl_TThreadImpl* pThreadImpl= (osl_TThreadImpl*)Thread;

    OSL_ASSERT(pDelay);

    /* invalid arguments?*/
    if(pThreadImpl==0 || pThreadImpl->m_ThreadId==-1)
        return osl_Thread_SleepError;

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SLEEP)
        return osl_Thread_SleepActive;

    DosEnterCritSec();

    if (pThreadImpl->m_hEvent == 0)
        rc = DosCreateEventSem( NULL,       /* unnamed semaphore */
                                &pThreadImpl->m_hEvent,  /* pointer to variable */
                                                   /* for the sem-handle */
                                DC_SEM_SHARED,  /* shared semaphore */
                                FALSE );         /* initial state is posted */
    else
        DosResetEventSem(pThreadImpl->m_hEvent, &ulPostCount);

    /* get thread ID */
    {
        PTIB pptib = NULL;
        PPIB pppib = NULL;

        DosGetInfoBlocks( &pptib, &pppib );
        tidCurrentThread = pptib->tib_ptib2->tib2_ultid;
    }

    millisecs = pDelay->Seconds * 1000 + pDelay->Nanosec / 1000000;

    if (pThreadImpl->m_ThreadId == tidCurrentThread)
    {
        pThreadImpl->m_Timeout = 0;
        pThreadImpl->m_Flags |=  THREADIMPL_FLAGS_SLEEP;

        DosExitCritSec();

        rc = DosWaitEventSem(pThreadImpl->m_hEvent, millisecs);

        DosEnterCritSec();

        pThreadImpl->m_Flags &=  ~THREADIMPL_FLAGS_SLEEP;

        DosExitCritSec();

        return (rc == ERROR_TIMEOUT) ? osl_Thread_SleepNormal :
                                       osl_Thread_SleepCancel;
    }
    else
    {
        pThreadImpl->m_Timeout = millisecs;
        pThreadImpl->m_Flags |=  THREADIMPL_FLAGS_SLEEP;

        DosExitCritSec();

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
    if (pThreadImpl==0 || pThreadImpl->m_ThreadId==-1)
    {
        /* assume thread is not running */
        return sal_False;
    }

    DosEnterCritSec();

    if (pThreadImpl->m_Flags & THREADIMPL_FLAGS_SLEEP)
    {
        DosPostEventSem(pThreadImpl->m_hEvent);

        DosExitCritSec();
        return sal_True;
    }
    else
    {
        DosExitCritSec();
        return sal_True;
    }
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

    DosEnterCritSec();
    pThreadImpl->m_Flags |= THREADIMPL_FLAGS_TERMINATE;
    DosExitCritSec();
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

        DosEnterCritSec();

        pThreadImpl->m_Timeout = 0;

        pThreadImpl->m_Flags &= ~THREADIMPL_FLAGS_SLEEP;

        DosExitCritSec();
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

/*****************************************************************************/
/* osl_createThreadKey */
/*****************************************************************************/
oslThreadKey SAL_CALL osl_createThreadKey(void)
{
    PULONG ptr;

    if (DosAllocThreadLocalMemory(1, &ptr) != NO_ERROR)
        ptr = NULL;
    else
        *ptr = 0L;

    return ((oslThreadKey)ptr);
}

/*****************************************************************************/
/* osl_destroyThreadKey */
/*****************************************************************************/
void SAL_CALL osl_destroyThreadKey(oslThreadKey Key)
{
    if (Key != 0L)
        DosFreeThreadLocalMemory((PULONG)Key);
}

/*****************************************************************************/
/* osl_getThreadKeyData */
/*****************************************************************************/
void * SAL_CALL osl_getThreadKeyData(oslThreadKey Key)
{
    if (Key != 0L)
        return ((void *)*((PULONG)Key));

    return (NULL);
}

/*****************************************************************************/
/* osl_setThreadKeyData */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setThreadKeyData(oslThreadKey Key, void *pData)
{
    if (Key != 0L)
        return *((PULONG)Key) = (ULONG)pData;

    return (sal_False);
}



