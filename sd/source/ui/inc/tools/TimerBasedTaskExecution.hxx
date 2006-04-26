/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TimerBasedTaskExecution.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-04-26 20:46:45 $
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

#ifndef SD_TIMER_BASED_TASK_EXECUTION_HXX
#define SD_TIMER_BASED_TASK_EXECUTION_HXX

#include <vcl/timer.hxx>

#include <boost/shared_ptr.hpp>

namespace sd { namespace tools {

class AsynchronousTask;

/** Execute an AsynchronousTask timer based, i.e. every
    nMillisecondsBetweenSteps milliseconds as much steps are executed as fit
    into a nMaxTimePerStep millisecond intervall.

    When a task is executed completely, i.e. HasNextStep() returns <FALSE/>,
    the TimerBasedTaskExecution destroys itself.  This, of course, works
    only if the creating instance does not hold a shared_ptr to  that object.
*/
class TimerBasedTaskExecution
{
public:
    /** Create a new object of this class.
        @param rpTask
            The AsynchronousTask that is to be executed.
        @param nMillisecondsBetweenSteps
            Wait at least this long between the execution of steps.  Note
            that more than one step may be executed in succession.
        @param nMaxTimePerStep
            The maximal time for executing steps without yielding control.
    */
    static ::boost::shared_ptr<TimerBasedTaskExecution> Create (
        const ::boost::shared_ptr<AsynchronousTask>& rpTask,
        sal_uInt32 nMillisecondsBetweenSteps,
        sal_uInt32 nMaxTimePerStep);

    /** Stop the execution of the task and release the shared pointer to
        itself so that it will eventually be destroyed.
    */
    void Release (void);

    /** Convenience method that calls Release() on the given task.  It
        checks the given weak_ptr for being expired and catches bad_weak_ptr
        exceptions.
    */
    static void ReleaseTask (const ::boost::weak_ptr<TimerBasedTaskExecution>& rpTask);

private:
    ::boost::shared_ptr<AsynchronousTask> mpTask;
    Timer maTimer;
    /** This shared_ptr to this is used to destroy a TimerBasedTaskExecution
        object when its task has been executed completely.
    */
    ::boost::shared_ptr<TimerBasedTaskExecution> mpSelf;
    sal_uInt32 mnMaxTimePerStep;

    TimerBasedTaskExecution (
        const ::boost::shared_ptr<AsynchronousTask>& rpTask,
        sal_uInt32 nMillisecondsBetweenSteps,
        sal_uInt32 nMaxTimePerStep);
    ~TimerBasedTaskExecution (void);
    void SetSelf (const ::boost::shared_ptr<TimerBasedTaskExecution>& rpSelf);

    class Deleter;
    friend class Deleter;

    DECL_LINK(TimerCallback,Timer*);
};

} } // end of namespace ::sd::tools

#endif
