/*************************************************************************
 *
 *  $RCSfile: mutex.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obr $ $Date: 2001-04-06 14:55:37 $
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

#ifdef PTHREAD_NONE_INIT
static pthread_t _pthread_none_ = PTHREAD_NONE_INIT;
#endif


/*
    Implementation notes:
    oslMutex hides a pointer to the oslMutexImpl structure, which
    ist needed to manage recursive locks on a mutex.

*/


#if !defined(PTHREAD_MUTEX_RECURSIVE) || !defined(USE_RECURSIVE_MUTEX)


/*
 *  mfe: if a Unix supports the recursive pthread mutex
 *       it can use the implementation beneath ...
 */


/*
 *  mfe: Default implementation of recursive mutexes
 */

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

        OSL_ASSERT( pthread_equal(pMutex->owner, PTHREAD_NONE) );

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

            OSL_ASSERT( pthread_equal(pMutex->owner, PTHREAD_NONE) );

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

#else

/*
 *  mfe: Recursive mutexes are supported from Solaris 2.7 on
 *       we should use them as the default
 *       But in the time being we don't use them ...
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

    nRet = pthread_mutexattr_settype(&aMutexAttr, PTHREAD_MUTEX_RECURSIVE);

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
        {
            OSL_TRACE("osl_tryToacquireMutex : mutex trylock failed. Errno: %d; %s\n",
                      nRet, strerror(nRet));
            return sal_False;
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

    OSL_ASSERT(pMutex);

    if ( pMutex )
    {
        int nRet=0;
        nRet = pthread_mutex_unlock(&(pMutex->mutex));
        if ( nRet != 0 )
        {
            OSL_TRACE("osl_releaseMutex : mutex unlock failed. Errno: %d; %s\n",
                      nRet, strerror(nRet));
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
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_NONE,
        0
    };

    /* necessary to get a "oslMutex *" */
    static oslMutex const globalMutex = (oslMutex) &globalMutex;

    return &globalMutex
}

#endif /* #if !defined (PTHREAD_MUTEX_RECURSIVE) */
