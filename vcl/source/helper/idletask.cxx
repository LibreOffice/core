/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/idletask.hxx>
#include <vcl/svapp.hxx>

//constructor of IdleTask Class
IdleTask::IdleTask()
    : flag(false)
{
    //setting the Priority of Idle task to TOOLKIT_DEBUG
    maIdle.SetPriority(TaskPriority::TOOLKIT_DEBUG);
    //set idle for callback
    maIdle.SetInvokeHandler(LINK(this, IdleTask, FlipFlag));
    //starting the idle
    maIdle.Start();
}

//GetFlag() of IdleTask Class
bool IdleTask::GetFlag() const
{
    //returning the status of current flag
    return flag;
}

//Callback function of IdleTask Class
IMPL_LINK(IdleTask, FlipFlag, Timer*, , void)
{
    //setting the flag to make sure that low priority idle task has been dispatched
    flag = true;
}

void IdleTask::waitUntilIdleDispatched()
{
    //creating instance of IdleTask Class
    IdleTask idleTask;
    while (!idleTask.GetFlag())
    {
        //dispatching all the events via VCL main-loop
        SolarMutexGuard aGuard;
        Application::Yield();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
