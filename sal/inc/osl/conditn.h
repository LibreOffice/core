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


#ifndef _OSL_CONDITION_H_
#define _OSL_CONDITION_H_

#include "sal/config.h"

#include "osl/time.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* oslCondition;

typedef enum {
    osl_cond_result_ok,     /* successful completion */
    osl_cond_result_error,  /* error occurred, check osl_getLastSocketError() for details */
    osl_cond_result_timeout, /* blocking operation timed out */
    osl_cond_result_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslConditionResult;

/** Creates a condition.
    The condition is in the reset-state.
    @returns 0 if condition could not be created.
*/
SAL_DLLPUBLIC oslCondition SAL_CALL osl_createCondition(void);

/** Free the memory used by the condition.
    @param Condition the condition handle.
*/
SAL_DLLPUBLIC void SAL_CALL osl_destroyCondition(oslCondition Condition);

/** Sets condition to True => wait() will not block, check() returns True.
    NOTE: ALL threads waiting on this condition are unblocked!
    @param Condition handle to a created condition.
    @return False if system-call failed.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_setCondition(oslCondition Condition);

/** Sets condition to False => wait() will block, check() returns False
    @param Condition handle to a created condition.
    @return False if system-call failed.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_resetCondition(oslCondition Condition);

/** Blocks if condition is not set<BR>
    If condition has been destroyed prematurely, wait() will
    return with False.
    @param Condition handle to a created condition.
    @param pTimeout Tiemout value or NULL for infinite waiting
    @return False if system-call failed.
*/
SAL_DLLPUBLIC oslConditionResult SAL_CALL osl_waitCondition(oslCondition Condition, const TimeValue* pTimeout);

/** Queries the state of the condition without blocking.
    @param Condition handle to a created condition.
    @return True: condition is set. <BR>
    False: condition is not set. <BR>
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_checkCondition(oslCondition Condition);

#ifdef __cplusplus
}
#endif

#endif /* _OSL_CONDITION_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
