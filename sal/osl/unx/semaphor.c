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
