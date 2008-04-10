/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: semaphor.h,v $
 * $Revision: 1.11 $
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

#ifndef _OSL_SEMAPHORE_H_
#define _OSL_SEMAPHORE_H_

#include <sal/types.h>

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


