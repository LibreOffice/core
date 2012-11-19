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

#include <osl/semaphor.h>
#include <osl/diagnose.h>

/* This is the (default) POSIX thread-local semaphore variant */

/*
    Implemetation notes:
    The void* represented by oslSemaphore is used
    as a pointer to an sem_t struct
*/

/*****************************************************************************/
/* osl_createSemaphore  */
/*****************************************************************************/

oslSemaphore SAL_CALL osl_createSemaphore(sal_uInt32 initialCount)
{
    int ret = 0;
    oslSemaphore Semaphore;

    Semaphore= malloc(sizeof(sem_t));

    OSL_ASSERT(Semaphore);      /* ptr valid? */

    if ( Semaphore == 0 )
    {
        return 0;
    }

    /* unnamed semaphore, not shared between processes */

       ret= sem_init((sem_t*)Semaphore, 0, initialCount);

    /* create failed? */
    if (ret != 0)
    {
        OSL_TRACE("osl_createSemaphore failed. Errno: %d; %s\n",
                  errno,
                  strerror(errno));

        free(Semaphore);
        Semaphore = NULL;
    }

    return Semaphore;
}

/*****************************************************************************/
/* osl_destroySemaphore  */
/*****************************************************************************/
void SAL_CALL osl_destroySemaphore(oslSemaphore Semaphore)
{
    if(Semaphore)           /* ptr valid? */
    {
        sem_destroy((sem_t*)Semaphore);
        free(Semaphore);
    }
}

/*****************************************************************************/
/* osl_acquireSemaphore  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireSemaphore(oslSemaphore Semaphore) {

    OSL_ASSERT(Semaphore != 0); /* abort in debug mode */

    if (Semaphore != 0)     /* be tolerant in release mode */
    {
        return (sem_wait((sem_t*)Semaphore) == 0);
    }

    return sal_False;
}

/*****************************************************************************/
/* osl_tryToAcquireSemaphore  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireSemaphore(oslSemaphore Semaphore) {

    OSL_ASSERT(Semaphore != 0); /* abort in debug mode */
    if (Semaphore != 0)     /* be tolerant in release mode */
    {
        return (sem_trywait((sem_t*)Semaphore) == 0);
    }

    return sal_False;
}

/*****************************************************************************/
/* osl_releaseSemaphore  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_releaseSemaphore(oslSemaphore Semaphore) {

    OSL_ASSERT(Semaphore != 0);     /* abort in debug mode */

    if (Semaphore != 0)         /* be tolerant in release mode */
    {
        return (sem_post((sem_t*)Semaphore) == 0);
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
