/*************************************************************************
 *
 *  $RCSfile: semaphor.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    int nCount;
    HEV hevReachedZero;
} oslSemaphoreImpl;

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
    pSemaphoreImpl = malloc(sizeof(oslSemaphoreImpl));
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

    DosEnterCritSec();

    while( pSemaphoreImpl->nCount < 1 )
    {
        sal_uInt32 nPostCount;

        DosExitCritSec();

        rc = DosWaitEventSem(pSemaphoreImpl->hevReachedZero, SEM_INDEFINITE_WAIT );
        DosResetEventSem(pSemaphoreImpl->hevReachedZero, &nPostCount);

        DosEnterCritSec();
    }

    pSemaphoreImpl->nCount--;
    DosExitCritSec();

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

    DosEnterCritSec();

    nCount = pSemaphoreImpl->nCount;
    if( pSemaphoreImpl->nCount > 0 )
        pSemaphoreImpl->nCount--;

    DosExitCritSec();

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

    DosEnterCritSec();

    nCount = pSemaphoreImpl->nCount;
    pSemaphoreImpl->nCount++;

    DosExitCritSec();

    if( nCount == 0 )
        DosPostEventSem(pSemaphoreImpl->hevReachedZero);

    return( rc == NO_ERROR );
}


