/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mutex.h,v $
 * $Revision: 1.9 $
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

#ifndef _OSL_MUTEX_H_
#define _OSL_MUTEX_H_

#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _oslMutexImpl;
typedef struct _oslMutexImpl * oslMutex;

/** Create a thread-local mutex.
    @return 0 if the mutex could not be created, otherwise a handle to the mutex.
*/
oslMutex SAL_CALL osl_createMutex(void);

/** Release the OS-structures and free mutex data-structure.
    @param Mutex the mutex-handle
*/
void SAL_CALL osl_destroyMutex(oslMutex Mutex);

/** Acquire the mutex, block if already acquired by another thread.
    @param Mutex handle to a created mutex.
    @return False if system-call fails.
*/
sal_Bool SAL_CALL osl_acquireMutex(oslMutex Mutex);

/** Try to acquire the mutex without blocking.
    @param Mutex handle to a created mutex.
    @return False if it could not be acquired.
*/
sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex Mutex);

/** Release the mutex.
    @param Mutex handle to a created mutex.
    @return False if system-call fails.
*/
sal_Bool SAL_CALL osl_releaseMutex(oslMutex Mutex);

/** Returns a unique and global mutex.
    @return the global mutex.
*/
oslMutex * SAL_CALL osl_getGlobalMutex(void);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_MUTEX_H_ */
