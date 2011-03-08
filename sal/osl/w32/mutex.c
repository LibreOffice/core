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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
