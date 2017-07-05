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

#include "system.h"

#include <osl/conditn.h>
#include <osl/diagnose.h>
#include <osl/time.h>

/*
    under WIN32, we use the void* oslCondition
    as a WIN32 HANDLE (which is also a 32-bit value)
*/

/*****************************************************************************/
/* osl_createCondition */
/*****************************************************************************/
oslCondition SAL_CALL osl_createCondition(void)
{
    oslCondition Condition;

    Condition= reinterpret_cast<oslCondition>(CreateEvent(nullptr,          /* no security */
                                         true,      /* manual reset */
                                         false,     /* initial state not signaled */
                                         nullptr));         /* automatic name */

    return Condition;

}

/*****************************************************************************/
/* osl_destroyCondition */
/*****************************************************************************/
void SAL_CALL osl_destroyCondition(oslCondition Condition)
{
    if(Condition)
    {
        OSL_VERIFY(CloseHandle(Condition));
    }
}

/*****************************************************************************/
/* osl_setCondition */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setCondition(oslCondition Condition)
{
    assert(Condition);

    return SetEvent(reinterpret_cast<HANDLE>(Condition)) != FALSE;
}

/*****************************************************************************/
/* osl_resetCondition */
/*****************************************************************************/
sal_Bool SAL_CALL osl_resetCondition(oslCondition Condition)
{
    assert(Condition);

    return ResetEvent(reinterpret_cast<HANDLE>(Condition)) != FALSE;
}

/*****************************************************************************/
/* osl_waitCondition */
/*****************************************************************************/
oslConditionResult SAL_CALL osl_waitCondition(oslCondition Condition,
                                     const TimeValue* pTimeout)
{
    DWORD timeout;

    assert(Condition);

    if (pTimeout)
        timeout = pTimeout->Seconds * 1000 + pTimeout->Nanosec / 1000000L;
    else
        timeout = INFINITE;

    /* It's necessary to process SendMessage calls to the current thread to give other threads
        access to COM objects instantiated in this thread */

    while ( true )
    {
        /* Only wake up if a SendMessage call to the threads message loop is detected */
        switch( MsgWaitForMultipleObjects( 1, reinterpret_cast<HANDLE *>(&Condition), FALSE, timeout, QS_SENDMESSAGE ) )
        {
            case WAIT_OBJECT_0 + 1:
                {
                MSG msg;

                /* We Must not dispatch the message. PM_NOREMOVE leaves the message queue untouched
                 but dispatches SendMessage calls automatically */

                PeekMessage( &msg, nullptr, 0, 0, PM_NOREMOVE );
                }
                break;

            case WAIT_OBJECT_0:
                return osl_cond_result_ok;

            case WAIT_TIMEOUT:
                return osl_cond_result_timeout;

            default:
                return osl_cond_result_error;
        }
    }
}

/*****************************************************************************/
/* osl_checkCondition */
/*****************************************************************************/
sal_Bool SAL_CALL osl_checkCondition(oslCondition Condition)
{
    assert(Condition);

    return WaitForSingleObject(reinterpret_cast<HANDLE>(Condition), 0) == WAIT_OBJECT_0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
