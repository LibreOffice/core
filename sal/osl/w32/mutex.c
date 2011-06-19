/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/*
    Implementation notes:
    The void* hidden by oslMutex points to a WIN32
    CRITICAL_SECTION structure.
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

    return (oslMutex)pMutexImpl;
}

/*****************************************************************************/
/* osl_destroyMutex */
/*****************************************************************************/
void SAL_CALL osl_destroyMutex(oslMutex Mutex)
{
    CRITICAL_SECTION *pMutexImpl = (CRITICAL_SECTION *)Mutex;

    if (pMutexImpl)
    {
        DeleteCriticalSection(pMutexImpl);
        free(pMutexImpl);
    }
}

/*****************************************************************************/
/* osl_acquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireMutex(oslMutex Mutex)
{
    CRITICAL_SECTION *pMutexImpl = (CRITICAL_SECTION *)Mutex;

    OSL_ASSERT(Mutex);

    EnterCriticalSection(pMutexImpl);

    return sal_True;
}

/*****************************************************************************/
/* osl_tryToAcquireMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex Mutex)
{
    CRITICAL_SECTION *pMutexImpl = (CRITICAL_SECTION *)Mutex;

    OSL_ASSERT(Mutex);

    return (sal_Bool)(TryEnterCriticalSection(pMutexImpl) != FALSE);
}

/*****************************************************************************/
/* osl_releaseMutex */
/*****************************************************************************/
sal_Bool SAL_CALL osl_releaseMutex(oslMutex Mutex)
{
    CRITICAL_SECTION *pMutexImpl = (CRITICAL_SECTION *)Mutex;

    OSL_ASSERT(Mutex);

    LeaveCriticalSection(pMutexImpl);

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
