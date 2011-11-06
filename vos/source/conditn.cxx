/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <osl/time.h>
#include <vos/conditn.hxx>
#include <vos/diagnose.hxx>

using namespace vos;

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OCondition, vos), VOS_NAMESPACE(OCondition, vos), VOS_NAMESPACE(OObject, vos), 0);

/// initial state of condition is not set
OCondition::OCondition()
{
    m_Condition= osl_createCondition();
}

OCondition::~OCondition()
{
    osl_destroyCondition(m_Condition);
}

/// set condition to sal_True => wait() will not block, check() returns sal_True
void OCondition::set()
{
    osl_setCondition(m_Condition);
}

/// set condition to sal_False => wait() will block, check() returns sal_False
void OCondition::reset()
{
    osl_resetCondition(m_Condition);
}

/** Blocks if condition is not set<BR>
    If condition has been destroyed prematurely, wait() will
    return with sal_False.
*/
OCondition::TResult OCondition::wait(const TimeValue* pTimeout)
{
    return (TResult)osl_waitCondition(m_Condition, pTimeout);
}

/** sal_True: condition is set <BR>
    sal_False: condition is not set <BR>
    does not block
*/
sal_Bool OCondition::check()
{
    return osl_checkCondition(m_Condition);
}

