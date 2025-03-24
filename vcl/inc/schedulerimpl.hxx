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

#pragma once

#include <vcl/scheduler.hxx>
#include <vcl/task.hxx>

class Task;

// Internal scheduler record holding intrusive linked list pieces
struct ImplSchedulerData final
{
    ImplSchedulerData* mpNext;        ///< Pointer to the next element in list
    Task*              mpTask;        ///< Pointer to VCL Task instance
    sal_uInt64         mnUpdateTime;  ///< Last Update Time
    TaskPriority       mePriority;    ///< Task priority
    /**
     * Is the Task currently processed / on the stack?
     *
     * Since the introduction of the scheduler stack, this became merely a
     * debugging and assertion hint. No decisions are anymore made based on
     * this, because invoked Tasks are removed from the scheduler lists and
     * placed on the stack, so no code should actually ever find one, where
     * mbInScheduler is true (I don't see a reason to walk the stack for
     * normal Scheduler usage, that is).
     *
     * This was originally used to prevent invoking Tasks recursively.
     **/
    bool               mbInScheduler; ///< Task currently processed?

    const char *GetDebugName() const;
};

class SchedulerGuard final
{
public:
    SchedulerGuard()
    {
        Scheduler::Lock();
    }

    ~SchedulerGuard()
    {
        Scheduler::Unlock();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
