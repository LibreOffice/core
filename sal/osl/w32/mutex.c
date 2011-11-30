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



#include "system.h"

#include <osl/mutex.h>
#include <osl/diagnose.h>

/*
    Implementation notes:
    The void* hidden by oslMutex points to a WIN32
    CRITICAL_SECTION structure.
*/

typedef struct _oslMutexImpl {
    CRITICAL_SECTION    m_Mutex;
    int                 m_Locks;
    DWORD               m_Owner;
    DWORD               m_Requests;
} oslMutexImpl;

static BOOL (WINAPI *lpfTryEnterCriticalSection)(LPCRITICAL_SECTION)
    = (BOOL (WINAPI *)(LPCRITICAL_SECTION))0xFFFFFFFF;

static CRITICAL_SECTION MutexLock;

/*****************************************************************************/
/* osl_createMutex */
/*****************************************************************************/
oslMutex SAL_CALL osl_createMutex(void)
{
    oslMutexImpl *pMutexImpl;

    /* Window 95 does not support "TryEnterCriticalSection" */

    if (lpfTryEnterCriticalSection ==
                (BOOL (WINAPI *)(LPCRITICAL_SECTION))0xFFFFFFFF)
    {
        OSVERSIONINFO VersionInformation =

        {
            sizeof(OSVERSIONINFO),
            0,
            0,
            0,
            0,
            "",
        };

        /* ts: Window 98 does not support "TryEnterCriticalSection" but export the symbol !!!
           calls to that symbol always returns FALSE */
        if (
            GetVersionEx(&VersionInformation) &&
            (VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT)
           )
        {
            lpfTryEnterCriticalSection = (BOOL (WINAPI *)(LPCRITICAL_SECTION))
                    GetProcAddress(GetModuleHandle("KERNEL32"),
                                   "TryEnterCriticalSection");
        }
        else
        {
            lpfTryEnterCriticalSection = (BOOL (WINAPI *)(LPCRITICAL_SECTION))NULL;
        }


        InitializeCriticalSection(&MutexLock);
    }

    pMutexImpl= calloc(sizeof(oslMutexImpl), 1);

    OSL_ASSERT(pMutexImpl); /* alloc successful? */

    InitializeCriticalSection(&pMutexImpl->m_Mutex);

    return (oslMutex)pMutexImpl;
}

/*****************************************************************************/
/* osl_destroyMutex */
/*****************************************************************************/
void SAL_CALL osl_destroyMutex(oslMutex Mutex)
{
    oslMutexImpl *pMutexImpl = (oslMutexImpl *)Mutex;

    if (pMutexImpl)
    {
        DeleteCriticalSection(&pMutexImpl->m_Mutex);
        free(pMutexImpl);
    }
}

/*****************************************************************************/
/* osl_acquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireMutex(oslMutex Mutex)
{
    oslMutexImpl *pMutexImpl = (oslMutexImpl *)Mutex;

    OSL_ASSERT(Mutex);

    if (lpfTryEnterCriticalSection == NULL)
    {
        EnterCriticalSection(&MutexLock);
        pMutexImpl->m_Requests++;
        LeaveCriticalSection(&MutexLock);

        EnterCriticalSection(&pMutexImpl->m_Mutex);

        EnterCriticalSection(&MutexLock);
        pMutexImpl->m_Requests--;
        if (pMutexImpl->m_Locks++ == 0)
            pMutexImpl->m_Owner = GetCurrentThreadId();
        LeaveCriticalSection(&MutexLock);
    }
    else
        EnterCriticalSection(&pMutexImpl->m_Mutex);

    return sal_True;
}

/*****************************************************************************/
/* osl_tryToAcquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex Mutex)
{
    sal_Bool     ret = sal_False;
    oslMutexImpl *pMutexImpl = (oslMutexImpl *)Mutex;

    OSL_ASSERT(Mutex);

    if (lpfTryEnterCriticalSection != NULL)
        return (sal_Bool)(lpfTryEnterCriticalSection(&pMutexImpl->m_Mutex) != FALSE);
    else
    {
        EnterCriticalSection(&MutexLock);

        if ( ((pMutexImpl->m_Requests == 0) && (pMutexImpl->m_Locks == 0)) ||
             (pMutexImpl->m_Owner == GetCurrentThreadId()) )
            ret = osl_acquireMutex(Mutex);

        LeaveCriticalSection(&MutexLock);
    }

    return ret;
}

/*****************************************************************************/
/* osl_releaseMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_releaseMutex(oslMutex Mutex)
{
    oslMutexImpl *pMutexImpl = (oslMutexImpl *)Mutex;

    OSL_ASSERT(Mutex);

    if (lpfTryEnterCriticalSection == NULL)
    {
        EnterCriticalSection(&MutexLock);

        if (--(pMutexImpl->m_Locks) == 0)
            pMutexImpl->m_Owner = 0;

        LeaveCriticalSection(&MutexLock);
    }

    LeaveCriticalSection(&pMutexImpl->m_Mutex);

    return sal_True;
}

/*****************************************************************************/
/* osl_getGlobalMutex */
/*****************************************************************************/

/* initialized in dllentry.c */
oslMutex g_Mutex;

oslMutex * SAL_CALL osl_getGlobalMutex(void)
{
    return &g_Mutex;
}
