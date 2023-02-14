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

#include <sal/config.h>

#include <assert.h>
#include <condition_variable>
#include <mutex>

#include <sal/log.hxx>
#include <sal/types.h>

#include <osl/conditn.h>
#include <osl/time.h>

namespace {

struct oslConditionImpl
{
    std::condition_variable  m_Condition;
    std::mutex      m_Lock;
    bool            m_State = false;
};

}

oslCondition SAL_CALL osl_createCondition()
{
    oslConditionImpl* pCond = new oslConditionImpl;

    SAL_INFO( "sal.osl.condition", "osl_createCondition(): " << pCond );

    return static_cast<oslCondition>(pCond);
}

void SAL_CALL osl_destroyCondition(oslCondition Condition)
{
    oslConditionImpl* pCond;

    pCond = static_cast<oslConditionImpl*>(Condition);

    SAL_INFO( "sal.osl.condition", "osl_destroyCondition(" << pCond << ")" );

    if ( pCond )
        delete pCond;
}

sal_Bool SAL_CALL osl_setCondition(oslCondition Condition)
{
   oslConditionImpl* pCond;

   assert(Condition);
   pCond = static_cast<oslConditionImpl*>(Condition);

   {
       std::unique_lock g(pCond->m_Lock);

       pCond->m_State = true;
       pCond->m_Condition.notify_all();
   }
   SAL_INFO( "sal.osl.condition", "osl_setCondition(" << pCond << ")" );

   return true;

}

sal_Bool SAL_CALL osl_resetCondition(oslCondition Condition)
{
    oslConditionImpl* pCond;

    assert(Condition);

    pCond = static_cast<oslConditionImpl*>(Condition);

    {
        std::unique_lock g(pCond->m_Lock);

        pCond->m_State = false;
    }
    SAL_INFO( "sal.osl.condition", "osl_resetCondition(" << pCond << ")" );

    return true;
}

oslConditionResult SAL_CALL osl_waitCondition(oslCondition Condition, const TimeValue* pTimeout)
{
    oslConditionImpl* pCond;

    assert(Condition);
    pCond = static_cast<oslConditionImpl*>(Condition);

    SAL_INFO( "sal.osl.condition", "osl_waitCondition(" << pCond << ")" );

    {
        std::unique_lock g(pCond->m_Lock);

        if ( pTimeout )
        {
            if ( ! pCond->m_State )
            {
                auto duration = std::chrono::seconds(pTimeout->Seconds)
                                + std::chrono::nanoseconds(pTimeout->Nanosec);
                if (!pCond->m_Condition.wait_for(g, duration, [&pCond](){return pCond->m_State;}))
                    return osl_cond_result_timeout;
            }
        }
        else
        {
            pCond->m_Condition.wait(g, [&pCond](){return pCond->m_State;});
        }
    }
    SAL_INFO( "sal.osl.condition", "osl_waitCondition(" << pCond << "): OK" );

    return osl_cond_result_ok;
}

sal_Bool SAL_CALL osl_checkCondition(oslCondition Condition)
{
    bool State;
    oslConditionImpl* pCond;

    assert(Condition);
    pCond = static_cast<oslConditionImpl*>(Condition);

    {
        std::unique_lock g(pCond->m_Lock);

        State = pCond->m_State;
    }
    SAL_INFO( "sal.osl.condition", "osl_checkCondition(" << pCond << "): " << (State ? "YES" : "NO") );

    return State;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
