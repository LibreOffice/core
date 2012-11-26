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



#ifndef SD_TIMER_BASED_TASK_EXECUTION_HXX
#define SD_TIMER_BASED_TASK_EXECUTION_HXX

#include <vcl/timer.hxx>

#include <boost/shared_ptr.hpp>

namespace sd { namespace tools {

class AsynchronousTask;

/** Execute an AsynchronousTask timer based, i.e. every
    nMillisecondsBetweenSteps milliseconds as much steps are executed as fit
    into a nMaxTimePerStep millisecond intervall.

    When a task is executed completely, i.e. HasNextStep() returns <false/>,
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
