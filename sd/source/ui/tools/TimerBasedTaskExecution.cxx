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
#include "precompiled_sd.hxx"

#include "tools/TimerBasedTaskExecution.hxx"
#include "tools/AsynchronousTask.hxx"
#include <tools/time.hxx>
#include <osl/diagnose.h>
#include <boost/weak_ptr.hpp>

#undef VERBOSE

namespace sd { namespace tools {

/** Used by the shared_ptr instead of the private destructor.
*/
class TimerBasedTaskExecution::Deleter
{
public:
    void operator() (TimerBasedTaskExecution* pObject)
    {
        delete pObject;
    }
};




::boost::shared_ptr<TimerBasedTaskExecution> TimerBasedTaskExecution::Create (
    const ::boost::shared_ptr<AsynchronousTask>& rpTask,
    sal_uInt32 nMillisecondsBetweenSteps,
    sal_uInt32 nMaxTimePerStep)
{
    ::boost::shared_ptr<TimerBasedTaskExecution> pExecution(
        new TimerBasedTaskExecution(rpTask,nMillisecondsBetweenSteps,nMaxTimePerStep),
        Deleter());
    // Let the new object have a shared_ptr to itself, so that it can
    // release itself when the AsynchronousTask has been executed
    // completely.
    pExecution->SetSelf(pExecution);
    return pExecution;
}




void TimerBasedTaskExecution::Release (void)
{
    maTimer.Stop();
    mpSelf.reset();
}




//static
void TimerBasedTaskExecution::ReleaseTask (
    const ::boost::weak_ptr<TimerBasedTaskExecution>& rpExecution)
{
    if ( ! rpExecution.expired())
    {
        try
        {
            ::boost::shared_ptr<tools::TimerBasedTaskExecution> pExecution (rpExecution);
            pExecution->Release();
        }
        catch (::boost::bad_weak_ptr)
        {
            // When a bad_weak_ptr has been thrown then the object pointed
            // to by rpTask has been released right after we checked that it
            // still existed.  Too bad, but that means, that we have nothing
            // more do.
        }
    }
}




TimerBasedTaskExecution::TimerBasedTaskExecution (
    const ::boost::shared_ptr<AsynchronousTask>& rpTask,
    sal_uInt32 nMillisecondsBetweenSteps,
    sal_uInt32 nMaxTimePerStep)
    : mpTask(rpTask),
      maTimer(),
      mpSelf(),
      mnMaxTimePerStep(nMaxTimePerStep)
{
    Link aLink(LINK(this,TimerBasedTaskExecution,TimerCallback));
    maTimer.SetTimeoutHdl(aLink);
    maTimer.SetTimeout(nMillisecondsBetweenSteps);
    maTimer.Start();
}




TimerBasedTaskExecution::~TimerBasedTaskExecution (void)
{
    maTimer.Stop();
}




void TimerBasedTaskExecution::SetSelf (
    const ::boost::shared_ptr<TimerBasedTaskExecution>& rpSelf)
{
    if (mpTask.get() != NULL)
        mpSelf = rpSelf;
}




IMPL_LINK(TimerBasedTaskExecution,TimerCallback, Timer*,EMPTYARG)
{
    if (mpTask.get() != NULL)
    {
        if (mpTask->HasNextStep())
        {
            // Execute as many steps as fit into the time span of length
            // mnMaxTimePerStep.  Note that the last step may take longer
            // than allowed.
            sal_uInt32 nStartTime (Time().GetMSFromTime());
#ifdef VERBOSE
            OSL_TRACE("starting TimerBasedTaskExecution at %d", nStartTime);
#endif
            do
            {
                mpTask->RunNextStep();
                sal_uInt32 nDuration (Time().GetMSFromTime()-nStartTime);
#ifdef VERBOSE
            OSL_TRACE("executed step in %d", nDuration);
#endif
                if (nDuration > mnMaxTimePerStep)
                    break;
            }
            while (mpTask->HasNextStep());
#ifdef VERBOSE
            OSL_TRACE("TimerBasedTaskExecution sleeping");
#endif
            maTimer.Start();
        }
        else
            mpSelf.reset();
    }

    return 0;
}


} } // end of namespace ::sd::tools

