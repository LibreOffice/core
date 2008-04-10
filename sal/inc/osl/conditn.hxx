/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: conditn.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _OSL_CONDITN_HXX_
#define _OSL_CONDITN_HXX_

#ifdef __cplusplus

#include <osl/time.h>

#include <osl/conditn.h>


namespace osl
{

    class Condition
    {
    public:

        enum Result
        {
            result_ok      = osl_cond_result_ok,
            result_error   = osl_cond_result_error,
            result_timeout = osl_cond_result_timeout
        };

        /* Create a condition.
         */
        Condition()
        {
            condition = osl_createCondition();
        }

        /* Release the OS-structures and free condition data-structure.
         */
        ~Condition()
        {
            osl_destroyCondition(condition);
        }

        /* Release all waiting threads, check returns sal_True.
         */
        void set()
        {
            osl_setCondition(condition);
        }

        /* Reset condition to false: wait() will block, check() returns sal_False.
         */
        void reset() {
            osl_resetCondition(condition);
        }

        /** Blocks the calling thread until condition is set.
         */
        Result wait(const TimeValue *pTimeout = 0)
        {
            return (Result) osl_waitCondition(condition, pTimeout);
        }

        /** Checks if the condition is set without blocking.
         */
        sal_Bool check()
        {
            return osl_checkCondition(condition);
        }


    private:
        oslCondition condition;

        /** The underlying oslCondition has no reference count.

        Since the underlying oslCondition is not a reference counted object, copy
        constructed Condition may work on an already destructed oslCondition object.

        */
        Condition(const Condition&);

        /** The underlying oslCondition has no reference count.

        When destructed, the Condition object destroys the undelying oslCondition,
        which might cause severe problems in case it's a temporary object.

        */
        Condition(oslCondition condition);

        /** This assignment operator is private for the same reason as
            the copy constructor.
        */
        Condition& operator= (const Condition&);

        /** This assignment operator is private for the same reason as
            the constructor taking a oslCondition argument.
        */
        Condition& operator= (oslCondition);
    };

}

#endif  /* __cplusplus */
#endif  /* _OSL_CONDITN_HXX_ */

