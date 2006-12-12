/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TimerBasedTaskExecution.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:53:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "tools/TimerBasedTaskExecution.hxx"
#include "tools/AsynchronousTask.hxx"

#ifndef _TIME_HXX
#include <tools/time.hxx>
#endif
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

    return 0;
}


} } // end of namespace ::sd::tools

