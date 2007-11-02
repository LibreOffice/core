/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mutex.c,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:31:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <sys/fmutex.h>

#include "system.h"

#include <osl/mutex.h>
#include <osl/diagnose.h>

/*
    Implementation notes:
    The void* hidden by oslMutex points to an OS/2 mutex semaphore.
*/
typedef struct _oslMutexImpl {
    HMTX                m_Mutex;
    int                 m_Locks;
    ULONG               m_Owner;
    ULONG               m_Requests;
} oslMutexImpl;

// static mutex to control access to private members of oslMutexImpl
static HMTX MutexLock = 0;

/*****************************************************************************/
/* osl_createMutex */
/*****************************************************************************/
oslMutex SAL_CALL osl_createMutex()
{
    oslMutexImpl *pMutexImpl;
    HMTX hMutex;
    APIRET rc;

    pMutexImpl= (oslMutexImpl*)calloc(sizeof(oslMutexImpl), 1);
    OSL_ASSERT(pMutexImpl); /* alloc successful? */

    /* create semaphore */
    rc = DosCreateMutexSem( NULL, &pMutexImpl->m_Mutex, 0, FALSE );
    if( rc != 0 )
    {
        free(pMutexImpl);
        return NULL;
    }

    // create static mutex for private members
    if (MutexLock == 0)
        DosCreateMutexSem( NULL, &MutexLock, 0, FALSE );

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
        DosCloseMutexSem( pMutexImpl->m_Mutex);
        free(pMutexImpl);
    }
}

/*****************************************************************************/
/* osl_acquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireMutex(oslMutex Mutex)
{
    oslMutexImpl *pMutexImpl = (oslMutexImpl *)Mutex;
    APIRET rc = 0;
    OSL_ASSERT(Mutex);

    DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );
    pMutexImpl->m_Requests++;
    DosReleaseMutexSem( MutexLock);

    rc = DosRequestMutexSem( pMutexImpl->m_Mutex, SEM_INDEFINITE_WAIT );

    DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );
    pMutexImpl->m_Requests--;
    if (pMutexImpl->m_Locks++ == 0)
        pMutexImpl->m_Owner = _gettid();
    DosReleaseMutexSem( MutexLock);

    return( rc == 0 );
}

/*****************************************************************************/
/* osl_tryToAcquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex Mutex)
{
    sal_Bool     ret = sal_False;
    oslMutexImpl *pMutexImpl = (oslMutexImpl *)Mutex;
    OSL_ASSERT(Mutex);

    DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );

    if ( ((pMutexImpl->m_Requests == 0) && (pMutexImpl->m_Locks == 0)) ||
         (pMutexImpl->m_Owner == _gettid()) )
        ret = osl_acquireMutex(Mutex);

    DosReleaseMutexSem( MutexLock);

    return ret;
}

/*****************************************************************************/
/* osl_releaseMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_releaseMutex(oslMutex Mutex)
{
    oslMutexImpl *pMutexImpl = (oslMutexImpl *)Mutex;
    APIRET rc;
    OSL_ASSERT(Mutex);

    DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );

    if (--(pMutexImpl->m_Locks) == 0)
        pMutexImpl->m_Owner = 0;

    DosReleaseMutexSem( MutexLock);

    rc = DosReleaseMutexSem( pMutexImpl->m_Mutex);

    return sal_True;
}



/*****************************************************************************/
/* osl_getGlobalMutex */
/*****************************************************************************/

oslMutex g_Mutex = NULL;

oslMutex * SAL_CALL osl_getGlobalMutex(void)
{
    if (g_Mutex == NULL)
        g_Mutex = osl_createMutex();
    return &g_Mutex;
}
