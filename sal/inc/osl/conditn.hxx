/*************************************************************************
 *
 *  $RCSfile: conditn.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_CONDITN_HXX_
#define _OSL_CONDITN_HXX_

#ifdef __cplusplus

#include <osl/macros.hxx>

#include <osl/conditn.h>


#ifdef _USE_NAMESPACE
namespace osl
{
#endif

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

        Condition(oslCondition condition)
        {
            this->condition = condition;
        }
    };

#ifdef _USE_NAMESPACE
}
#endif

#endif  /* __cplusplus */
#endif  /* _OSL_CONDITN_HXX_ */

