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



#ifndef __FRAMEWORK_HELPER_TIMERHELPER_HXX_
#define __FRAMEWORK_HELPER_TIMERHELPER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <vos/timer.hxx>
#include <osl/mutex.hxx>

//........................................................................
namespace framework
{
//........................................................................

    //====================================================================
    //= ITimerListener
    //====================================================================
    class ITimerListener
    {
    public:
        virtual void    timerExpired() = 0;
    };
//........................................................................
}   // namespace framework
//........................................................................

#endif // _FRAMEWORK_HELPER_TIMERHELPER_HXX_

