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
