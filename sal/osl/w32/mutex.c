/*************************************************************************
 *
 *  $RCSfile: mutex.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obr $ $Date: 2001-04-06 14:32:01 $
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
oslMutex SAL_CALL osl_createMutex()
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
        return lpfTryEnterCriticalSection(&pMutexImpl->m_Mutex);
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

oslMutex * SAL_CALL osl_getGlobalMutex()
{
    return &g_Mutex;
}
