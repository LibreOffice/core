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



#ifndef _OSL_SEMAPHORE_H_
#define _OSL_SEMAPHORE_H_

#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* oslSemaphore;

/** Creates a semaphore.<BR>

    @deprecated
    Must not be used, as unnamed semaphores are not supported on Mac OS X.

    @param InitialCount denotes the starting value the semaphore. If you set it to
    zero, the first acquire() blocks. Otherwise InitialCount acquire()s  are
    immedeatly  successful.
    @return 0 if the semaphore could not be created, otherwise a handle to the sem.
*/
oslSemaphore SAL_CALL osl_createSemaphore(sal_uInt32 initialCount);

/** Release the OS-structures and free semaphore data-structure

    @deprecated
    Must not be used, as unnamed semaphores are not supported on Mac OS X.

    @return fbbb
*/
void SAL_CALL osl_destroySemaphore(oslSemaphore Semaphore);

/** acquire() decreases the count. It will block if it tries to
    decrease below zero.

    @deprecated
    Must not be used, as unnamed semaphores are not supported on Mac OS X.

    @return False if the system-call failed.
*/
sal_Bool SAL_CALL osl_acquireSemaphore(oslSemaphore Semaphore);

/** tryToAcquire() tries to decreases the count. It will
    return with False if it would decrease the count below zero.
    (When acquire() would block.) If it could successfully
    decrease the count, it will return True.

    @deprecated
    Must not be used, as unnamed semaphores are not supported on Mac OS X.
*/
sal_Bool SAL_CALL osl_tryToAcquireSemaphore(oslSemaphore Semaphore);

/** release() increases the count.

    @deprecated
    Must not be used, as unnamed semaphores are not supported on Mac OS X.

    @return False if the system-call failed.
*/
sal_Bool SAL_CALL osl_releaseSemaphore(oslSemaphore Semaphore);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_SEMAPHORE_H_  */


