/*************************************************************************
 *
 *  $RCSfile: mutex.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:21 $
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


#if defined(SOLARIS)

#include <sys/utsname.h>
#include <string.h>

volatile int GlobalInit = 55;

void ChangeGlobalInit()
{
    GlobalInit = 0;
}

pthread_t faked_pthread_self ()
{
    if (GlobalInit == 55)
        return (pthread_t)0;
    else
        return pthread_self();
}

#define pthread_self() faked_pthread_self()

#endif /* SOLARIS */


/*
    Implementation notes:
    oslMutex hides a pointer to the MutexContext structure, which
    ist needed to manage recursive locks on a mutex.

*/


typedef struct _oslMutexImpl
{
    pthread_mutex_t mutex;
    pthread_t       owner;
    sal_uInt32      locks;
} MutexContext;

/*****************************************************************************/
/* osl_createMutex */
/*****************************************************************************/
oslMutex SAL_CALL osl_createMutex()
{
    MutexContext* pMutex = (MutexContext*) malloc(sizeof(MutexContext));
    int nRet=0;

    OSL_ASSERT(pMutex);

    if ( pMutex == 0 )
    {
        return 0;
    }

    nRet = pthread_mutex_init(&(pMutex->mutex), PTHREAD_MUTEXATTR_DEFAULT);
    if ( nRet != 0 )
    {
        OSL_TRACE("osl_createMutex : mutex init failed. Errno: %d; %s\n",
                  nRet, strerror(nRet));

        free(pMutex);
        return 0;
    }

    pMutex->owner = PTHREAD_NONE;
    pMutex->locks =  0;

    return (oslMutex) pMutex;
}

/*****************************************************************************/
/* osl_destroyMutex */
/*****************************************************************************/
void SAL_CALL osl_destroyMutex(oslMutex Mutex)
{
    MutexContext* pMutex = (MutexContext*) Mutex;

    OSL_ASSERT(pMutex);

    if ( pMutex != NULL ) /* is critical section valid? */
    {
        int nRet=0;

#if defined(SOLARIS)
        if ( GlobalInit != 55 )
        {
#endif
            OSL_ASSERT( pthread_equal(pMutex->owner, PTHREAD_NONE) );
#if defined(SOLARIS)
        }
#endif

        nRet = pthread_mutex_destroy(&(pMutex->mutex));
        if ( nRet != 0 )
        {
            OSL_TRACE("osl_destroyMutex : mutex destroy failed. Errno: %d; %s\n",
                      nRet, strerror(nRet));
        }

        free(pMutex);
    }

    return;
}

/*****************************************************************************/
/* osl_acquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireMutex(oslMutex Mutex)
{
    MutexContext* pMutex = (MutexContext*) Mutex;
    int nRet=0;

    OSL_ASSERT(pMutex);

    if ( pMutex )
    {
        pthread_t thread_id = pthread_self();

        if ( pthread_equal(pMutex->owner, thread_id ) )
        {
            pMutex->locks++;
/*            fprintf(stderr,"osl_acquireMutex 0x%08X (locks == %i) from %i to thread %i (lock inc)\n",
              pMutex,pMutex->locks,pMutex->owner,pthread_self());*/
        }
        else
        {
            nRet = pthread_mutex_lock(&(pMutex->mutex));
            if ( nRet != 0 )
            {
                OSL_TRACE("osl_acquireMutex : mutex lock failed. Errno: %d; %s\n",
                          nRet, strerror(nRet));
            }

/*            fprintf(stderr,"osl_acquireMutex 0x%08X (locks == %i) from %i to thread %i\n",
              pMutex,pMutex->locks,pMutex->owner,thread_id);*/

            OSL_ASSERT( pMutex->locks ==  0 );

            pMutex->owner = thread_id;
        }

        return sal_True;
    }

    /* not initialized */
    return sal_False;
}

/*****************************************************************************/
/* osl_tryToAcquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex Mutex)
{
    MutexContext* pMutex = (MutexContext*) Mutex;

    OSL_ASSERT(pMutex);

    if ( pMutex )
    {
        int nRet = 0;

        if ( pthread_equal(pMutex->owner, pthread_self()) )
        {
            pMutex->locks++;
        }
        else
        {
            nRet = pthread_mutex_trylock(&(pMutex->mutex));
            if ( nRet != 0  )
            {
                OSL_TRACE("osl_tryToacquireMutex : mutex trylock failed. Errno: %d; %s\n",
                          nRet, strerror(nRet));
                return sal_False;
            }

#if defined(SOLARIS)
            if ( GlobalInit != 55 )
            {
#endif
                OSL_ASSERT( pthread_equal(pMutex->owner, PTHREAD_NONE) );
#if defined(SOLARIS)
            }
#endif

            OSL_ASSERT( pMutex->locks ==  0 );

            pMutex->owner = pthread_self();
        }

        return sal_True;
    }

    /* not initialized */
    return sal_False;
}

/*****************************************************************************/
/* osl_releaseMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_releaseMutex(oslMutex Mutex)
{
    MutexContext* pMutex = (MutexContext*) Mutex;
    int nRet=0;

    OSL_ASSERT(pMutex);

    if ( pMutex )
    {
        if ( pthread_equal( pMutex->owner, pthread_self()) )
        {
            if ( pMutex->locks > 0 )
            {
                pMutex->locks--;
/*                fprintf(stderr,"osl_releaseMutex 0x%08X (locks == %i) from %i to thread %i (lock dec)\n",
                  pMutex,pMutex->locks,pMutex->owner,pthread_self());*/
            }
            else
            {
/*                fprintf(stderr,"osl_releaseMutex 0x%08X (locks == %i) from %i to thread %i\n",
                  pMutex,pMutex->locks,pMutex->owner,pthread_self());*/

                pMutex->owner = PTHREAD_NONE;

                nRet = pthread_mutex_unlock(&(pMutex->mutex));
                if ( nRet != 0 )
                {
                    OSL_TRACE("osl_releaseMutex : mutex unlock failed. Errno: %d; %s\n",
                              nRet, strerror(nRet));
                }
            }

            return (sal_True);
        }
        else
        {
/*            fprintf(stderr,"osl_releaseMutex 0x%08X (locks == %i) from %i to thread %i (not owner)\n",
              pMutex,pMutex->locks,pMutex->owner,pthread_self());*/

            nRet = pthread_mutex_unlock(&(pMutex->mutex));
            if ( nRet != 0 )
            {
                    OSL_TRACE("osl_releaseMutex : mutex unlock failed. Errno: %d; %s\n",
                              nRet, strerror(nRet));
                    return sal_False;
            }

            return sal_True;
        }

    }

    /* not initialized */
    return sal_False;
}



