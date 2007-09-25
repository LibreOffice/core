/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mutex.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-25 09:48:14 $
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

#include "system.h"

#include <osl/mutex.h>
#include <osl/diagnose.h>

/*
#include <windows.h>
#include <winbase.h>
#include <malloc.h>
*/

/*
    Implementation notes:
    The void* hidden by oslMutex points to a WIN32
    CRITICAL_SECTION structure.
*/
typedef struct _oslMutexImpl {
    HMTX                m_Mutex;
    int                 m_Locks;
    ULONG               m_Owner;
    ULONG               m_Requests;
} oslMutexImpl;

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
//printf( "osl_createMutex pid %d(0x%04x), tid %d\n", getpid(), getpid(), _gettid());
//printf( "&pMutexImpl->m_Mutex %08x\n", &pMutexImpl->m_Mutex);
//printf( "pMutexImpl->m_Mutex %d\n", pMutexImpl->m_Mutex);
//printf( "DosCreateMutexSem rc=%d\n", rc);
    if( rc != NO_ERROR )
    {
        free(pMutexImpl);
        return NULL;
    }

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
//printf( "osl_destroyMutex pid %d(0x%04x), tid %d\n", getpid(), getpid(), _gettid());
//printf( "pMutexImpl->m_Mutex %d\n", pMutexImpl->m_Mutex);
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
    APIRET rc;
    OSL_ASSERT(Mutex);

//printf( "osl_acquireMutex pid %d(0x%04x), tid %d\n", getpid(), getpid(), _gettid());
//printf( "pMutexImpl->m_Mutex %d\n", pMutexImpl->m_Mutex);

    DosEnterCritSec();
    pMutexImpl->m_Requests++;
    DosExitCritSec();

    rc = DosRequestMutexSem( pMutexImpl->m_Mutex, SEM_INDEFINITE_WAIT );

    DosEnterCritSec();
    pMutexImpl->m_Requests--;
    if (pMutexImpl->m_Locks++ == 0)
        pMutexImpl->m_Owner = _gettid();
    DosExitCritSec();

    return( rc == NO_ERROR );
}

/*****************************************************************************/
/* osl_tryToAcquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex Mutex)
{
    sal_Bool     ret = sal_False;
    oslMutexImpl *pMutexImpl = (oslMutexImpl *)Mutex;

    OSL_ASSERT(Mutex);

    DosEnterCritSec();

    if ( ((pMutexImpl->m_Requests == 0) && (pMutexImpl->m_Locks == 0)) ||
         (pMutexImpl->m_Owner == _gettid()) )
        ret = osl_acquireMutex(Mutex);

    DosExitCritSec();

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

    DosEnterCritSec();

    if (--(pMutexImpl->m_Locks) == 0)
        pMutexImpl->m_Owner = 0;

    DosExitCritSec();

    rc = DosReleaseMutexSem( pMutexImpl->m_Mutex);

    return( rc == NO_ERROR );
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
