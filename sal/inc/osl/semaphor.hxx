/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
            immedeatly  successfull.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
