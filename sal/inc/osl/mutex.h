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
