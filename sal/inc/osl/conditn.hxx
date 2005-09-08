/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conditn.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:26:24 $
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

#ifndef _OSL_CONDITN_HXX_
#define _OSL_CONDITN_HXX_

#ifdef __cplusplus

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

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

