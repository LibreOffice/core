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



#ifndef _OSL_SEMAPHORE_HXX_
#define _OSL_SEMAPHORE_HXX_

#ifdef __cplusplus

#include <osl/semaphor.h>


namespace osl
{
    /** C++ wrapper class around C semaphore functions.

        @deprecated
        Must not be used, as unnamed semaphores are not supported on Mac OS X.
    */
    class Semaphore {

    public:

        /** Creates a semaphore.<BR>
            @param InitialCount denotes the starting value the semaphore. If you set it to
            zero, the first acquire() blocks. Otherwise InitialCount acquire()s  are
            immediately successful.
            @return 0 if the semaphore could not be created, otherwise a handle to the sem.
        */

        Semaphore(sal_uInt32 initialCount)
        {
            semaphore = osl_createSemaphore(initialCount);
        }

        /** Release the OS-structures and free semaphore data-structure
            @return fbbb
        */
        ~Semaphore()
        {
            osl_destroySemaphore(semaphore);
        }

        /** acquire() decreases the count. It will block if it tries to
            decrease below zero.
            @return False if the system-call failed.
        */
        sal_Bool acquire()
        {
            return osl_acquireSemaphore(semaphore);
        }

        /** tryToAcquire() tries to decreases the count. It will
            return with False if it would decrease the count below zero.
            (When acquire() would block.) If it could successfully
            decrease the count, it will return True.
        */
        sal_Bool tryToAcquire()
        {
            return osl_tryToAcquireSemaphore(semaphore);
        }

        /** release() increases the count.
            @return False if the system-call failed.
        */
        sal_Bool release()
        {
            return osl_releaseSemaphore(semaphore);
        }

    private:
        oslSemaphore semaphore;

        /** The underlying oslSemaphore has no reference count.

        Since the underlying oslSemaphore is not a reference counted object, copy
        constructed Semaphore may work on an already destructed oslSemaphore object.

        */
        Semaphore(const Semaphore&);

        /** The underlying oslSemaphore has no reference count.

        When destructed, the Semaphore object destroys the undelying oslSemaphore,
        which might cause severe problems in case it's a temporary object.

        */
        Semaphore(oslSemaphore Semaphore);

        /** This assignment operator is private for the same reason as
            the copy constructor.
        */
        Semaphore& operator= (const Semaphore&);

        /** This assignment operator is private for the same reason as
            the constructor taking a oslSemaphore argument.
        */
        Semaphore& operator= (oslSemaphore);
    };
}

#endif  /* __cplusplus */
#endif  /* _OSL_SEMAPHORE_HXX_  */
