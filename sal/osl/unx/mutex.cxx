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

#if defined LINUX
// to define __USE_UNIX98, via _XOPEN_SOURCE, enabling pthread_mutexattr_settype
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#endif
#include "system.hxx"
#include "unixerrnostring.hxx"

#include <sal/log.hxx>
#include <osl/mutex.h>

#include <pthread.h>
#include <stdlib.h>


typedef struct _oslMutexImpl
{
    pthread_mutex_t mutex;
} oslMutexImpl;

oslMutex SAL_CALL osl_createMutex()
{
    oslMutexImpl* pMutex = static_cast<oslMutexImpl*>(malloc(sizeof(oslMutexImpl)));
    pthread_mutexattr_t aMutexAttr;
    int nRet=0;

    SAL_WARN_IF(!pMutex, "sal.osl.mutex", "null pMutex");

    if ( pMutex == nullptr )
    {
        return nullptr;
    }

    pthread_mutexattr_init(&aMutexAttr);

    nRet = pthread_mutexattr_settype(&aMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    if( nRet == 0 )
        nRet = pthread_mutex_init(&(pMutex->mutex), &aMutexAttr);
    if ( nRet != 0 )
    {
        SAL_WARN("sal.osl.mutex", "pthread_muxex_init failed: " << UnixErrnoString(nRet));

        free(pMutex);
        pMutex = nullptr;
    }

    pthread_mutexattr_destroy(&aMutexAttr);

    return pMutex;
}

void SAL_CALL osl_destroyMutex(oslMutex pMutex)
{
    SAL_WARN_IF(!pMutex, "sal.osl.mutex", "null pMutex");

    if ( pMutex != nullptr )
    {
        int nRet=0;

        nRet = pthread_mutex_destroy(&(pMutex->mutex));
        if ( nRet != 0 )
        {
            SAL_WARN("sal.osl.mutex", "pthread_mutex_destroy failed: " << UnixErrnoString(nRet));
        }

        free(pMutex);
    }
}

sal_Bool SAL_CALL osl_acquireMutex(oslMutex pMutex)
{
    SAL_WARN_IF(!pMutex, "sal.osl.mutex", "null pMutex");

    if ( pMutex != nullptr )
    {
        int nRet=0;

        nRet = pthread_mutex_lock(&(pMutex->mutex));
        if ( nRet != 0 )
        {
            SAL_WARN("sal.osl.mutex", "pthread_mutex_lock failed: " << UnixErrnoString(nRet));
            return false;
        }
        return true;
    }

    /* not initialized */
    return false;
}

sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex pMutex)
{
    bool result = false;

    SAL_WARN_IF(!pMutex, "sal.osl.mutex", "null pMutex");

    if ( pMutex )
    {
        int nRet = 0;
        nRet = pthread_mutex_trylock(&(pMutex->mutex));
        if ( nRet == 0  )
            result = true;
    }

    return result;
}

sal_Bool SAL_CALL osl_releaseMutex(oslMutex pMutex)
{
    SAL_WARN_IF(!pMutex, "sal.osl.mutex", "null pMutex");

    if ( pMutex )
    {
        int nRet=0;
        nRet = pthread_mutex_unlock(&(pMutex->mutex));
        if ( nRet != 0 )
        {
            SAL_WARN("sal.osl.mutex", "pthread_mutex_unlock failed: " << UnixErrnoString(nRet));
            return false;
        }

        return true;
    }

    /* not initialized */
    return false;
}

static oslMutexImpl globalMutexImpl;

static void globalMutexInitImpl() {
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0 ||
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) ||
        pthread_mutex_init(&globalMutexImpl.mutex, &attr) != 0 ||
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
