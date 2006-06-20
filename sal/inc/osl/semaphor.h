/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: semaphor.h,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:13:05 $
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

#ifndef _OSL_SEMAPHORE_H_
#define _OSL_SEMAPHORE_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* oslSemaphore;

/** Creates a semaphore.<BR>
    @param InitialCount denotes the starting value the semaphore. If you set it to
    zero, the first acquire() blocks. Otherwise InitialCount acquire()s  are
    immedeatly  successfull.
    @return 0 if the semaphore could not be created, otherwise a handle to the sem.
*/
oslSemaphore SAL_CALL osl_createSemaphore(sal_uInt32 initialCount);

/** Release the OS-structures and free semaphore data-structure
    @return fbbb
*/
void SAL_CALL osl_destroySemaphore(oslSemaphore Semaphore);

/** acquire() decreases the count. It will block if it tries to
    decrease below zero.
    @return False if the system-call failed.
*/
sal_Bool SAL_CALL osl_acquireSemaphore(oslSemaphore Semaphore);

/** tryToAcquire() tries to decreases the count. It will
    return with False if it would decrease the count below zero.
    (When acquire() would block.) If it could successfully
    decrease the count, it will return True.
*/
sal_Bool SAL_CALL osl_tryToAcquireSemaphore(oslSemaphore Semaphore);

/** release() increases the count.
    @return False if the system-call failed.
*/
sal_Bool SAL_CALL osl_releaseSemaphore(oslSemaphore Semaphore);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_SEMAPHORE_H_  */


