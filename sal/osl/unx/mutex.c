/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mutex.c,v $
 * $Revision: 1.15 $
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

#include <pthread.h>

/*
    Implementation notes:
    oslMutex hides a pointer to the oslMutexImpl structure, which
    ist needed to manage recursive locks on a mutex.

*/


/*#if defined(PTHREAD_MUTEX_RECURSIVE) && defined(USE_RECURSIVE_MUTEX)*/
#ifdef LINUX

// The obsolete pthread_mutex_setkind_np (used below in osl_createMutex) seems
// to be missing from pthread.h on LINUX:
int pthread_mutexattr_setkind_np(pthread_mutexattr_t *, int);

/*
 *  Linux fully supports recursive mutexes, but only with
 *  a non-standard function. So we linux, we use system recursive mutexes.
 */

typedef struct _oslMutexImpl
{
    pthread_mutex_t mutex;
} oslMutexImpl;


/*****************************************************************************/
/* osl_createMutex */
/*****************************************************************************/
oslMutex SAL_CALL osl_createMutex()
{
    oslMutexImpl* pMutex = (oslMutexImpl*) malloc(sizeof(oslMutexImpl));
    pthread_mutexattr_t aMutexAttr;
    int nRet=0;

    OSL_ASSERT(pMutex);

    if ( pMutex == 0 )
    {
        return 0;
    }

    pthread_mutexattr_init(&aMutexAttr);

//    nRet = pthread_mutexattr_settype(&aMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    nRet = pthread_mutexattr_setkind_np( &aMutexAttr, PTHREAD_MUTEX_RECURSIVE_NP );

    nRet = pthread_mutex_init(&(pMutex->mutex), &aMutexAttr);
    if ( nRet != 0 )
    {
        OSL_TRACE("osl_createMutex : mutex init failed. Errno: %d; %s\n",
                  nRet, strerror(nRet));

        free(pMutex);
        pMutex = 0;
    }

    pthread_mutexattr_destroy(&aMutexAttr);

    return (oslMutex) pMutex;
}

/*****************************************************************************/
/* osl_destroyMutex */
/*****************************************************************************/
void SAL_CALL osl_destroyMutex(oslMutex Mutex)
{
    oslMutexImpl* pMutex = (oslMutexImpl*) Mutex;

    OSL_ASSERT(pMutex);

    if ( pMutex != 0 )
    {
        int nRet=0;

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
    oslMutexImpl* pMutex = (oslMutexImpl*) Mutex;

    OSL_ASSERT(pMutex);

    if ( pMutex != 0 )
    {
        int nRet=0;

        nRet = pthread_mutex_lock(&(pMutex->mutex));
        if ( nRet != 0 )
        {
            OSL_TRACE("osl_acquireMutex : mutex lock failed. Errno: %d; %s\n",
                      nRet, strerror(nRet));
            return sal_False;
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
    oslMutexImpl* pMutex = (oslMutexImpl*) Mutex;

    OSL_ASSERT(pMutex);

    if ( pMutex )
    {
        int nRet = 0;
        nRet = pthread_mutex_trylock(&(pMutex->mutex));
        if ( nRet != 0  )
            return sal_False;

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
    oslMutexImpl* pMutex = (oslMutexImpl*) Mutex;

    OSL_ASSERT(pMutex);

    if ( pMutex )
    {
        int nRet=0;
        nRet = pthread_mutex_unlock(&(pMutex->mutex));
        if ( nRet != 0 )
        {
            OSL_TRACE("osl_releaseMutex : mutex unlock failed. Errno: %d; %s\n",
                      nRet, strerror(nRet));
            return sal_False;
        }

        return sal_True;
    }

    /* not initialized */
    return sal_False;
}

/*****************************************************************************/
/* osl_getGlobalMutex */
/*****************************************************************************/

oslMutex * SAL_CALL osl_getGlobalMutex()
{
    /* the static global mutex instance */
    static oslMutexImpl globalMutexImpl = {
        PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
    };

    /* necessary to get a "oslMutex *" */
    static oslMutex globalMutex = (oslMutex) &globalMutexImpl;

    return &globalMutex;
}

#else /* SOLARIS and all other platforms, use an own implementation of
         resursive mutexes */
/*
 *  jbu: Currently not used for solaris. The default implementation is buggy,
 *       the system needs to be patched to
 *                    SPARC       X86
 *       Solaris 7: 106980-17   106981-18
 *       Solaris 8: 108827-11   108828-11
 *
 *       As the performace advantage is not too high, it was decided to wait
 *       some more time. Note also, that solaris 2.6 does not support
 *       them at all.
 */

#ifdef PTHREAD_NONE_INIT
static pthread_t _pthread_none_ = PTHREAD_NONE_INIT;
#endif

typedef struct _oslMutexImpl
{
    pthread_mutex_t mutex;
    pthread_t       owner;
    sal_uInt32      locks;
} oslMutexImpl;


/*****************************************************************************/
/* osl_createMutex */
/*****************************************************************************/
oslMutex SAL_CALL osl_createMutex()
{
    oslMutexImpl* pMutex = (oslMutexImpl*) malloc(sizeof(oslMutexImpl));
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
    oslMutexImpl* pMutex = (oslMutexImpl*) Mutex;

    OSL_ASSERT(pMutex);

    if ( pMutex != NULL )
    {
        int nRet=0;

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
    oslMutexImpl* pMutex = (oslMutexImpl*) Mutex;

    OSL_ASSERT(pMutex);

    if ( pMutex )
    {
        int nRet=0;

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
    oslMutexImpl* pMutex = (oslMutexImpl*) Mutex;

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
    oslMutexImpl* pMutex = (oslMutexImpl*) Mutex;
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

/*****************************************************************************/
/* osl_getGlobalMutex */
/*****************************************************************************/

oslMutex * SAL_CALL osl_getGlobalMutex()
{
    /* the static global mutex instance */
    static oslMutexImpl globalMutexImpl = {
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_NONE_INIT,
        0
    };

    /* necessary to get a "oslMutex *" */
    static oslMutex globalMutex = (oslMutex) &globalMutexImpl;

    return &globalMutex;
}


#endif /* #ifndef LINUX */

