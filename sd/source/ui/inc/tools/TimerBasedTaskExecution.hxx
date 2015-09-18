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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TOOLS_TIMERBASEDTASKEXECUTION_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TOOLS_TIMERBASEDTASKEXECUTION_HXX

#include <vcl/timer.hxx>

#include <memory>

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
    static std::shared_ptr<TimerBasedTaskExecution> Create (
        const std::shared_ptr<AsynchronousTask>& rpTask,
        sal_uInt32 nMillisecondsBetweenSteps,
        sal_uInt32 nMaxTimePerStep);

    /** Stop the execution of the task and release the shared pointer to
        itself so that it will eventually be destroyed.
    */
    void Release();

    /** Convenience method that calls Release() on the given task.  It
        checks the given weak_ptr for being expired and catches bad_weak_ptr
        exceptions.
    */
    static void ReleaseTask (const std::weak_ptr<TimerBasedTaskExecution>& rpTask);

private:
    std::shared_ptr<AsynchronousTask> mpTask;
    Timer maTimer;
    /** This shared_ptr to this is used to destroy a TimerBasedTaskExecution
        object when its task has been executed completely.
    */
    std::shared_ptr<TimerBasedTaskExecution> mpSelf;
    sal_uInt32 mnMaxTimePerStep;

    TimerBasedTaskExecution (
        const std::shared_ptr<AsynchronousTask>& rpTask,
        sal_uInt32 nMillisecondsBetweenSteps,
        sal_uInt32 nMaxTimePerStep);
    ~TimerBasedTaskExecution();
    void SetSelf (const std::shared_ptr<TimerBasedTaskExecution>& rpSelf);

    class Deleter;
    friend class Deleter;

    DECL_LINK_TYPED(TimerCallback, Timer *, void);
};

} } // end of namespace ::sd::tools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
