/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mutex.h,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:29:10 $
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

#ifndef _OSL_MUTEX_H_
#define _OSL_MUTEX_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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
