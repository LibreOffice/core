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
    to store a OS/2 HANDLE.
*/

typedef struct _oslSemaphoreImpl
{
    HEV hevReachedZero;
    int nCount;
} oslSemaphoreImpl;

// static mutex to control access to private members of oslMutexImpl
static HMTX MutexLock = NULL;

/*****************************************************************************/
/* osl_createSemaphore */
/*****************************************************************************/

/*
- Erzeugen der Semaphore
- Z„hler auf initialCount setzen
*/
oslSemaphore SAL_CALL osl_createSemaphore(sal_uInt32 initialCount)
{
    APIRET rc;
    oslSemaphoreImpl * pSemaphoreImpl;

    /* alloc mem. for our internal data structure */
    pSemaphoreImpl = (oslSemaphoreImpl *) malloc(sizeof(oslSemaphoreImpl));
    if( pSemaphoreImpl == NULL )
        return NULL;

    /* create semaphore */
    rc = DosCreateEventSem( NULL,
                            &pSemaphoreImpl->hevReachedZero,
                            DC_SEM_SHARED,
                            FALSE );
    if( rc != NO_ERROR )
    {
        free( pSemaphoreImpl );
        return NULL;
    }

    pSemaphoreImpl->nCount = initialCount;

    // create static mutex for private members
    if (MutexLock == NULL)
        DosCreateMutexSem( NULL, &MutexLock, 0, FALSE );

    return (oslSemaphore) pSemaphoreImpl;
}

/*****************************************************************************/
/* osl_destroySemaphore */
/*****************************************************************************/

/*
- Semaphore l”schen
*/

void SAL_CALL osl_destroySemaphore(oslSemaphore Semaphore)
{
    oslSemaphoreImpl* pSemaphoreImpl = (oslSemaphoreImpl*)Semaphore;
    OSL_ASSERT(Semaphore != 0);

    DosCloseEventSem( pSemaphoreImpl->hevReachedZero );

    free( pSemaphoreImpl );
}

/*****************************************************************************/
/* osl_acquireSemaphore */
/*****************************************************************************/
/*
- Z„hler -1
- wenn Z„hler < 0: blockieren
*/

sal_Bool SAL_CALL osl_acquireSemaphore(oslSemaphore Semaphore)
{
    APIRET rc;
    oslSemaphoreImpl* pSemaphoreImpl = (oslSemaphoreImpl*)Semaphore;
    int nCount;
    OSL_ASSERT(Semaphore != 0);

    DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );

    while( pSemaphoreImpl->nCount < 1 )
    {
        sal_uInt32 nPostCount;

        DosReleaseMutexSem( MutexLock);

        rc = DosWaitEventSem(pSemaphoreImpl->hevReachedZero, SEM_INDEFINITE_WAIT );
        DosResetEventSem(pSemaphoreImpl->hevReachedZero, &nPostCount);

        DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );
    }

    pSemaphoreImpl->nCount--;
    DosReleaseMutexSem( MutexLock);

    return( rc == NO_ERROR );
}

/*****************************************************************************/
/* osl_tryToAcquireSemaphore */
/*****************************************************************************/
/*
- Z„hler -1, wenn vorher > 0
- wenn Z„hler < 0: mit FALSE zurueck
*/
sal_Bool SAL_CALL osl_tryToAcquireSemaphore(oslSemaphore Semaphore)
{
    APIRET rc;
    oslSemaphoreImpl* pSemaphoreImpl = (oslSemaphoreImpl*)Semaphore;
    int nCount;
    OSL_ASSERT(Semaphore != 0);

    DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );

    nCount = pSemaphoreImpl->nCount;
    if( pSemaphoreImpl->nCount > 0 )
        pSemaphoreImpl->nCount--;

    DosReleaseMutexSem( MutexLock);

    return( nCount > 0 );
}

/*****************************************************************************/
/* osl_releaseSemaphore */
/*****************************************************************************/
/*
- Z„hler +1
*/
sal_Bool SAL_CALL osl_releaseSemaphore(oslSemaphore Semaphore)
{
    APIRET rc;
    oslSemaphoreImpl* pSemaphoreImpl = (oslSemaphoreImpl*)Semaphore;
    int nCount;
    OSL_ASSERT(Semaphore != 0);

    DosRequestMutexSem( MutexLock, SEM_INDEFINITE_WAIT );

    nCount = pSemaphoreImpl->nCount;
    pSemaphoreImpl->nCount++;

    DosReleaseMutexSem( MutexLock);

    if( nCount == 0 )
        DosPostEventSem(pSemaphoreImpl->hevReachedZero);

    return( rc == NO_ERROR );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
