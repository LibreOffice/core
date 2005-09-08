/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: semaphor.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:32:32 $
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

#ifndef _OSL_SEMAPHORE_HXX_
#define _OSL_SEMAPHORE_HXX_

#ifdef __cplusplus

#include <osl/semaphor.h>


namespace osl
{

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
