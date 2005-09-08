/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: semaphor.c,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:11:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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



