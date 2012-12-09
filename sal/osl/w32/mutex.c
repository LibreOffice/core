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

/*
    Implementation notes:
    The oslMutex type is a typedef to CRITICAL_SECTION* .
*/

/*****************************************************************************/
/* osl_createMutex */
/*****************************************************************************/
oslMutex SAL_CALL osl_createMutex(void)
{
    CRITICAL_SECTION *pMutexImpl;

    pMutexImpl = calloc(sizeof(CRITICAL_SECTION), 1);

    OSL_ASSERT(pMutexImpl); /* alloc successful? */

    InitializeCriticalSection(pMutexImpl);

    return pMutexImpl;
}

sal_Bool SAL_CALL osl_initializeMutex(oslMutex pMutex)
{

    OSL_ASSERT(pMutexImpl);

    if (pMutex)
    {
        InitializeCriticalSection(pMutexImpl);

        return sal_True;
    }
    return sal_False;
}

/*****************************************************************************/
/* osl_destroyMutex */
/*****************************************************************************/
void SAL_CALL osl_destroyMutex(oslMutex pMutex)
{
    if (pMutex)
    {
        DeleteCriticalSection(pMutex);
        free(pMutex);
    }
}

void SAL_CALL osl_clearMutex(oslMutex pMutex)
{
    if (pMutex)
    {
        DeleteCriticalSection(pMutex);
    }
}

/*****************************************************************************/
/* osl_acquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireMutex(oslMutex Mutex)
{
    OSL_ASSERT(pMutex);

    EnterCriticalSection(pMutex);

    return sal_True;
}

/*****************************************************************************/
/* osl_tryToAcquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex pMutex)
{
    OSL_ASSERT(pMutex);

    return (sal_Bool)(TryEnterCriticalSection(pMutex) != FALSE);
}

/*****************************************************************************/
/* osl_releaseMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_releaseMutex(oslMutex pMutex)
{
    OSL_ASSERT(pMutex);

    LeaveCriticalSection(pMutex);

    return sal_True;
}

/*****************************************************************************/
/* osl_getGlobalMutex */
/*****************************************************************************/

/* initialized in dllentry.c */
oslMutex g_Mutex;

oslMutex * SAL_CALL osl_getGlobalMutex(void)
{
    return &g_Mutex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
