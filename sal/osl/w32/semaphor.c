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

#include <osl/diagnose.h>
#include <osl/semaphor.h>

/*
    Implemetation notes:
    The void* represented by oslSemaphore is used
    to store a WIN32 HANDLE.
*/


/*****************************************************************************/
/* osl_createSemaphore */
/*****************************************************************************/
oslSemaphore SAL_CALL osl_createSemaphore(sal_uInt32 initialCount)
{
    oslSemaphore Semaphore;

    Semaphore= CreateSemaphore(0, initialCount, INT_MAX, 0);

    /* create failed? */
    if((HANDLE)Semaphore == INVALID_HANDLE_VALUE)
    {
        Semaphore= 0;
    }

    return Semaphore;
}

/*****************************************************************************/
/* osl_destroySemaphore */
/*****************************************************************************/
void SAL_CALL osl_destroySemaphore(oslSemaphore Semaphore)
{


    if(Semaphore != 0)
    {
        CloseHandle((HANDLE)Semaphore);
    }

}

/*****************************************************************************/
/* osl_acquireSemaphore */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireSemaphore(oslSemaphore Semaphore)
{
    OSL_ASSERT(Semaphore != 0);

    switch ( WaitForSingleObject( (HANDLE)Semaphore, INFINITE ) )
    {
        case WAIT_OBJECT_0:
            return sal_True;

        default:
            return (sal_False);
    }
}

/*****************************************************************************/
/* osl_tryToAcquireSemaphore */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireSemaphore(oslSemaphore Semaphore)
{
    OSL_ASSERT(Semaphore != 0);
    return (sal_Bool)(WaitForSingleObject((HANDLE)Semaphore, 0) == WAIT_OBJECT_0);
}


/*****************************************************************************/
/* osl_releaseSemaphore */
/*****************************************************************************/
sal_Bool SAL_CALL osl_releaseSemaphore(oslSemaphore Semaphore)
{
    OSL_ASSERT(Semaphore != 0);

    /* increase count by one, not interested in previous count */
    return (sal_Bool)(ReleaseSemaphore((HANDLE)Semaphore, 1, NULL) != FALSE);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
