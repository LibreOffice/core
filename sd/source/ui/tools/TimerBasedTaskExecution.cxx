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

#include <tools/TimerBasedTaskExecution.hxx>
#include <tools/AsynchronousTask.hxx>
#include <tools/time.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <memory>

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

std::shared_ptr<TimerBasedTaskExecution> TimerBasedTaskExecution::Create (
    const std::shared_ptr<AsynchronousTask>& rpTask,
    sal_uInt32 nMillisecondsBetweenSteps,
    sal_uInt32 nMaxTimePerStep)
{
    std::shared_ptr<TimerBasedTaskExecution> pExecution(
        new TimerBasedTaskExecution(rpTask,nMillisecondsBetweenSteps,nMaxTimePerStep),
        Deleter());
    // Let the new object have a shared_ptr to itself, so that it can
    // release itself when the AsynchronousTask has been executed
    // completely.
    if (pExecution->mpTask != nullptr)
        pExecution->mpSelf = pExecution;
    return pExecution;
}

void TimerBasedTaskExecution::Release()
{
    maTimer.Stop();
    mpSelf.reset();
}

//static
void TimerBasedTaskExecution::ReleaseTask (
    const std::weak_ptr<TimerBasedTaskExecution>& rpExecution)
{
    if (  rpExecution.expired())
        return;

    try
    {
        std::shared_ptr<tools::TimerBasedTaskExecution> pExecution (rpExecution);
        pExecution->Release();
    }
    catch (const std::bad_weak_ptr&)
    {
        // When a bad_weak_ptr has been thrown then the object pointed
        // to by rpTask has been released right after we checked that it
        // still existed.  Too bad, but that means, that we have nothing
        // more do.
    }
}

TimerBasedTaskExecution::TimerBasedTaskExecution (
    const std::shared_ptr<AsynchronousTask>& rpTask,
    sal_uInt32 nMillisecondsBetweenSteps,
    sal_uInt32 nMaxTimePerStep)
    : mpTask(rpTask),
      maTimer(),
      mpSelf(),
      mnMaxTimePerStep(nMaxTimePerStep)
{
    maTimer.SetInvokeHandler( LINK(this,TimerBasedTaskExecution,TimerCallback) );
    maTimer.SetTimeout(nMillisecondsBetweenSteps);
    maTimer.Start();
}

TimerBasedTaskExecution::~TimerBasedTaskExecution()
{
    maTimer.Stop();
}

IMPL_LINK_NOARG(TimerBasedTaskExecution, TimerCallback, Timer *, void)
{
    if (mpTask == nullptr)
        return;

    if (mpTask->HasNextStep())
    {
        // Execute as many steps as fit into the time span of length
        // mnMaxTimePerStep.  Note that the last step may take longer
        // than allowed.
        sal_uInt32 nStartTime (::tools::Time( ::tools::Time::SYSTEM ).GetMSFromTime());
        SAL_INFO("sd.tools", OSL_THIS_FUNC << ": starting TimerBasedTaskExecution at " << nStartTime);
        do
        {
            mpTask->RunNextStep();
            sal_uInt32 nDuration (::tools::Time( ::tools::Time::SYSTEM ).GetMSFromTime()-nStartTime);
            SAL_INFO("sd.tools", OSL_THIS_FUNC << ": executed step in " << nDuration);
            if (nDuration > mnMaxTimePerStep)
                break;
        }
        while (mpTask->HasNextStep());
        SAL_INFO("sd.tools", OSL_THIS_FUNC << ": TimerBasedTaskExecution sleeping");
        maTimer.Start();
    }
    else
        mpSelf.reset();
}

} } // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
