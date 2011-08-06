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
