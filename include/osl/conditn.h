/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifndef INCLUDED_OSL_CONDITN_H
#define INCLUDED_OSL_CONDITN_H

#include <sal/config.h>

#include <osl/time.h>
#include <sal/saldllapi.h>

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
    @param pTimeout Timeout value or NULL for infinite waiting
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

#endif // INCLUDED_OSL_CONDITN_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
