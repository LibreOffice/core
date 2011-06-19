/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
        catch (const ::boost::bad_weak_ptr&)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
