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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_______________________________________________
// include files of own module
#include <helper/wakeupthread.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// declarations

//***********************************************
WakeUpThread::WakeUpThread(const css::uno::Reference< css::util::XUpdatable >& xListener)
    : ThreadHelpBase(         )
    , m_xListener   (xListener)
{
}

//***********************************************
void SAL_CALL WakeUpThread::run()
{
    ::osl::Condition aSleeper;

    TimeValue aTime;
    aTime.Seconds = 0;
    aTime.Nanosec = 25000000; // 25 msec

    while(schedule())
    {
        aSleeper.reset();
        aSleeper.wait(&aTime);

        // SAFE ->
        ReadGuard aReadLock(m_aLock);
        css::uno::Reference< css::util::XUpdatable > xListener(m_xListener.get(), css::uno::UNO_QUERY);
        aReadLock.unlock();
        // <- SAFE

        if (xListener.is())
            xListener->update();
    }
}

//***********************************************
void SAL_CALL WakeUpThread::onTerminated()
{
    delete this;
}

} // namespace framework
