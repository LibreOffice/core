/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


