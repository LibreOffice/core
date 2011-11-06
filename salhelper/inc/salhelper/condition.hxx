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




#ifndef _SALHELPER_CONDITION_HXX_
#define _SALHELPER_CONDITION_HXX_


#include <osl/conditn.h>
#include <osl/mutex.hxx>


namespace salhelper
{
    class ConditionModifier;
    class ConditionWaiter;


    class Condition
    {
        friend class ConditionModifier;
        friend class ConditionWaiter;

    public:

        Condition(osl::Mutex& aMutex);

        virtual ~Condition();


    protected:

        virtual bool applies() const = 0;


    private:
        Condition(Condition &); // not defined
        void operator =(Condition &); // not defined

        osl::Mutex&  m_aMutex;
        oslCondition m_aCondition;
    };



    class ConditionModifier
    {
    public:

        ConditionModifier(Condition& aCond);

        ~ConditionModifier();


    private:
        ConditionModifier(ConditionModifier &); // not defined
        void operator =(ConditionModifier &); // not defined

        Condition& m_aCond;
    };



    class ConditionWaiter
    {
    public:

        ConditionWaiter(Condition& aCond);

        struct timedout {
            timedout();

            timedout(timedout const &);

            virtual ~timedout();

            timedout & operator =(timedout const &);
        };

        ConditionWaiter(Condition& aCond,sal_uInt32 milliSec)
            throw(
                timedout
            );


        ~ConditionWaiter();


    private:
        ConditionWaiter(ConditionWaiter &); // not defined
        void operator =(ConditionWaiter &); // not defined

        Condition& m_aCond;
    };


}   // namespace salhelper


#endif
