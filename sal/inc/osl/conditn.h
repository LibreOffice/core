/*************************************************************************
 *
 *  $RCSfile: conditn.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-16 13:01:52 $
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


#ifndef _OSL_CONDITION_H_
#define _OSL_CONDITION_H_

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* oslCondition;

typedef enum {
    osl_cond_result_ok,     /* successful completion */
    osl_cond_result_error,  /* error occured, check osl_getLastSocketError() for details */
    osl_cond_result_timeout, /* blocking operation timed out */
    osl_cond_result_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslConditionResult;

/** Creates a condition.
    @returns 0 if condition could not be created.
*/
oslCondition SAL_CALL osl_createCondition(void);

/** Free the memory used by the condition.
    @param Condition the condition handle.
*/
void SAL_CALL osl_destroyCondition(oslCondition Condition);

/** Sets condition to True => wait() will not block, check() returns True.
    NOTE: ALL threads waiting on this condition are unblocked!
    @param Condition handle to a created condition.
    @return False if system-call failed.
*/
sal_Bool SAL_CALL osl_setCondition(oslCondition Condition);

/** Sets condition to False => wait() will block, check() returns False
    @param Condition handle to a created condition.
    @return False if system-call failed.
*/
sal_Bool SAL_CALL osl_resetCondition(oslCondition Condition);

/** Blocks if condition is not set<BR>
    If condition has been destroyed prematurely, wait() will
    return with False.
    @param Condition handle to a created condition.
    @param pTimeout Tiemout value or NULL for infinite waiting
    @return False if system-call failed.
*/
oslConditionResult SAL_CALL osl_waitCondition(oslCondition Condition, const TimeValue* pTimeout);

/** Queries the state of the condition without blocking.
    @param Condition handle to a created condition.
    @return True: condition is set. <BR>
    False: condition is not set. <BR>
*/
sal_Bool SAL_CALL osl_checkCondition(oslCondition Condition);

#ifdef __cplusplus
}
#endif

#endif /* _OSL_CONDITION_H_ */

