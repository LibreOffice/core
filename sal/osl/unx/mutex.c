/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "system.h"

#include <osl/mutex.h>
#include <osl/diagnose.h>

#include <pthread.h>
#include <stdlib.h>

#if defined LINUX /* bad hack */
int pthread_mutexattr_setkind_np(pthread_mutexattr_t *, int);
#define pthread_mutexattr_settype pthread_mutexattr_setkind_np
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif

/*****************************************************************************/
/* osl_createMutex */
/*****************************************************************************/
oslMutex SAL_CALL osl_createMutex()
{
    oslMutex pMutex = (oslMutex) malloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_t aMutexAttr;
    int nRet=0;

    OSL_ASSERT(pMutex);

    if ( pMutex == 0 )
    {
        return 0;
    }

    pthread_mutexattr_init(&aMutexAttr);

    nRet = pthread_mutexattr_settype(&aMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    if( nRet == 0 )
        nRet = pthread_mutex_init(pMutex, &aMutexAttr);
    if ( nRet != 0 )
    {
        OSL_TRACE("osl_createMutex : mutex init/setattr failed. Errno: %d; %s\n",
                  nRet, strerror(nRet));

        free(pMutex);
        pMutex = 0;
    }

    pthread_mutexattr_destroy(&aMutexAttr);

    return pMutex;
}

sal_Bool SAL_CALL osl_initializeMutex(oslMutex pMutex)
{
    sal_Bool retValue = sal_True;
    pthread_mutexattr_t aMutexAttr;
    int nRet=0;

    OSL_ASSERT(pMutex);

    if ( pMutex == 0 )
    {
        return sal_False;
    }

    pthread_mutexattr_init(&aMutexAttr);

    nRet = pthread_mutexattr_settype(&aMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    if( nRet == 0 )
        nRet = pthread_mutex_init(pMutex, &aMutexAttr);
    if ( nRet != 0 )
    {
        OSL_TRACE("osl_createMutex : mutex init/setattr failed. Errno: %d; %s\n",
                  nRet, strerror(nRet));

        pMutex = 0;
        retValue = sal_False;
    }

    pthread_mutexattr_destroy(&aMutexAttr);

    return retValue;
}

/*****************************************************************************/
/* osl_destroyMutex */
/*****************************************************************************/
void SAL_CALL osl_destroyMutex(oslMutex pMutex)
{
    OSL_ASSERT(pMutex);

    if ( pMutex != 0 )
    {
        int nRet=0;

        nRet = pthread_mutex_destroy(pMutex);
        if ( nRet != 0 )
        {
            OSL_TRACE("osl_destroyMutex : mutex destroy failed. Errno: %d; %s\n",
                      nRet, strerror(nRet));
        }

        free(pMutex);
    }
}

void SAL_CALL osl_clearMutex(oslMutex pMutex)
{
    OSL_ASSERT(pMutex);

    if ( pMutex != 0 )
    {
        int nRet=0;

        nRet = pthread_mutex_destroy(pMutex);
        if ( nRet != 0 )
        {
            OSL_TRACE("osl_destroyMutex : mutex destroy failed. Errno: %d; %s\n",
                      nRet, strerror(nRet));
        }
    }
}

/*****************************************************************************/
/* osl_acquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireMutex(oslMutex pMutex)
{
    OSL_ASSERT(pMutex);

    if ( pMutex != 0 )
    {
        int nRet=0;

        nRet = pthread_mutex_lock(pMutex);
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
sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex pMutex)
{
    OSL_ASSERT(pMutex);

    if ( pMutex )
    {
        int nRet = 0;
        nRet = pthread_mutex_trylock(pMutex);
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
sal_Bool SAL_CALL osl_releaseMutex(oslMutex pMutex)
{
    OSL_ASSERT(pMutex);

    if ( pMutex )
    {
        int nRet=0;
        nRet = pthread_mutex_unlock(pMutex);
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

static oslMutex_t globalMutexImpl;

static void globalMutexInitImpl(void) {
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0 ||
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) ||
        pthread_mutex_init(&globalMutexImpl, &attr) != 0 ||
        pthread_mutexattr_destroy(&attr) != 0)
    {
        abort();
    }
}

oslMutex * SAL_CALL osl_getGlobalMutex()
{
    /* necessary to get a "oslMutex *" */
    static oslMutex globalMutex = &globalMutexImpl;

    static pthread_once_t once = PTHREAD_ONCE_INIT;
    if (pthread_once(&once, &globalMutexInitImpl) != 0) {
        abort();
    }

    return &globalMutex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
