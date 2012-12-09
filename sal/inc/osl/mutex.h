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

#ifndef _OSL_MUTEX_H_
#define _OSL_MUTEX_H_

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SAL_W32)

#include "system.h"

typedef CRITICAL_SECTION oslMutex_t;

#elif defined(SAL_UNX)

#include <pthread.h>

typedef pthread_mutex_t oslMutex_t;

#endif

typedef oslMutex_t* oslMutex;

/** Create a thread-local mutex.
    @return 0 if the mutex could not be created, otherwise a handle to the mutex.
*/
SAL_DLLPUBLIC oslMutex SAL_CALL osl_createMutex(void);

/** Construct a new mutex.
    @param Mutex Mutex to initialize
    @return False if the Mutex could not be initialized
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_initializeMutex(oslMutex pMutex);

/** Release the OS-structures and free mutex data-structure.
    @param Mutex the mutex-handle
*/
SAL_DLLPUBLIC void SAL_CALL osl_destroyMutex(oslMutex Mutex);

/** Release the os-structures of the mutex.
    @param Mutex the mutex-handle
*/
SAL_DLLPUBLIC void SAL_CALL osl_clearMutex(oslMutex pMutex);


/** Acquire the mutex, block if already acquired by another thread.
    @param Mutex handle to a created mutex.
    @return False if system-call fails.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_acquireMutex(oslMutex Mutex);

/** Try to acquire the mutex without blocking.
    @param Mutex handle to a created mutex.
    @return False if it could not be acquired.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex Mutex);

/** Release the mutex.
    @param Mutex handle to a created mutex.
    @return False if system-call fails.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_releaseMutex(oslMutex Mutex);

/** Returns a unique and global mutex.
    @return the global mutex.
*/
SAL_DLLPUBLIC oslMutex * SAL_CALL osl_getGlobalMutex(void);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_MUTEX_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
