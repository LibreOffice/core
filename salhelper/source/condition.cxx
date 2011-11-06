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




#include <salhelper/condition.hxx>
#include <osl/time.h>


using namespace salhelper;


/******************************************************************
 *                                                                *
 *                     Condition                                  *
 *                                                                *
 ******************************************************************/

Condition::Condition(osl::Mutex& aMutex)
    : m_aMutex(aMutex),
      m_aCondition(osl_createCondition())
{
}


Condition::~Condition()
{
    osl_destroyCondition(m_aCondition);
}


/******************************************************************
 *                                                                *
 *              ConditionModifier                                 *
 *                                                                *
 ******************************************************************/

ConditionModifier::ConditionModifier(Condition& aCond)
    : m_aCond(aCond)
{
    m_aCond.m_aMutex.acquire();
}


ConditionModifier::~ConditionModifier()
{
    if(m_aCond.applies())
        osl_setCondition(m_aCond.m_aCondition);

    m_aCond.m_aMutex.release();
}



/******************************************************************
 *                                                                *
 *              ConditionWaiter                                   *
 *                                                                *
 ******************************************************************/

ConditionWaiter::timedout::timedout() {}

ConditionWaiter::timedout::timedout(timedout const &) {}

ConditionWaiter::timedout::~timedout() {}

ConditionWaiter::timedout &
ConditionWaiter::timedout::operator =(timedout const &) { return *this; }

ConditionWaiter::ConditionWaiter(Condition& aCond)
    : m_aCond(aCond)
{
    while(true) {
        osl_waitCondition(m_aCond.m_aCondition,0);
        m_aCond.m_aMutex.acquire();

        if(m_aCond.applies())
            break;
        else {
            osl_resetCondition(m_aCond.m_aCondition);
            m_aCond.m_aMutex.release();
        }
    }
}


ConditionWaiter::ConditionWaiter(Condition& aCond,sal_uInt32 milliSec)
    throw(
        ConditionWaiter::timedout
    )
    : m_aCond(aCond)
{
    TimeValue aTime;
    aTime.Seconds = milliSec / 1000;
    aTime.Nanosec = 1000000 * ( milliSec % 1000 );

    while(true) {
        if( osl_waitCondition(m_aCond.m_aCondition,&aTime) ==
            osl_cond_result_timeout )
            throw timedout();

        m_aCond.m_aMutex.acquire();

        if(m_aCond.applies())
            break;
        else {
            osl_resetCondition(m_aCond.m_aCondition);
            m_aCond.m_aMutex.release();
        }
    }
}


ConditionWaiter::~ConditionWaiter()
{
    if(! m_aCond.applies())
        osl_resetCondition(m_aCond.m_aCondition);
    m_aCond.m_aMutex.release();
}
