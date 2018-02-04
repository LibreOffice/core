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

#include <sal/config.h>

#include "gmutex.hxx"
#include "system.h"

#include <osl/mutex.h>
#include <osl/diagnose.h>

/*
    Implementation notes:
    The void* hidden by oslMutex points to a WIN32
    SRWLOCK structure.
*/

oslMutex SAL_CALL osl_createMutex(void)
{
    PSRWLOCK pMutexImpl;

    pMutexImpl = static_cast<PSRWLOCK>(calloc(sizeof(PSRWLOCK), 1));

    OSL_ASSERT(pMutexImpl); /* alloc successful? */

    InitializeSRWLock(pMutexImpl);

    return reinterpret_cast<oslMutex>(pMutexImpl);
}

void SAL_CALL osl_destroyMutex(oslMutex Mutex)
{
    // SRW locks do not need to be explicitly destroyed
    // (see https://msdn.microsoft.com/en-us/library/ms683483)
    // so only free the memory
    free(Mutex);
}

sal_Bool SAL_CALL osl_acquireMutex(oslMutex Mutex)
{
    PSRWLOCK pMutexImpl = reinterpret_cast<PSRWLOCK>(Mutex);

    OSL_ASSERT(Mutex);

    AcquireSRWLockExclusive(pMutexImpl);

    return true;
}

sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex Mutex)
{
    PSRWLOCK pMutexImpl = reinterpret_cast<PSRWLOCK>(Mutex);

    OSL_ASSERT(Mutex);

    return TryAcquireSRWLockExclusive(pMutexImpl) != FALSE;
}

sal_Bool SAL_CALL osl_releaseMutex(oslMutex Mutex)
{
    PSRWLOCK pMutexImpl = reinterpret_cast<PSRWLOCK>(Mutex);

    OSL_ASSERT(Mutex);

    ReleaseSRWLockExclusive(pMutexImpl);

    return true;
}

/* initialized in dllentry.c */
oslMutex g_Mutex;

oslMutex * SAL_CALL osl_getGlobalMutex(void)
{
    return &g_Mutex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
