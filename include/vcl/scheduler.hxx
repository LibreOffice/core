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

#ifndef INCLUDED_VCL_SCHEDULER_HXX
#define INCLUDED_VCL_SCHEDULER_HXX

#include <vcl/dllapi.h>

struct ImplSchedulerContext;

class VCL_DLLPUBLIC Scheduler final
{
    friend class SchedulerGuard;
    friend class Task;
    Scheduler() = delete;

    static inline void UpdateSystemTimer( ImplSchedulerContext &rSchedCtx,
                                          sal_uInt64 nMinPeriod,
                                          bool bForce, sal_uInt64 nTime );

    static void ImplStartTimer ( sal_uInt64 nMS, bool bForce, sal_uInt64 nTime );

    static void Lock();
    static void Unlock();

public:
    static constexpr sal_uInt64 ImmediateTimeoutMs = 0;
    static constexpr sal_uInt64 InfiniteTimeoutMs  = SAL_MAX_UINT64;

    static void       ImplDeInitScheduler();

    /**
     * System timer callback function, which processes one LO task
     *
     * Will restart the system timer, so it will process further tasks at the right time.
     **/
    static void CallbackTaskScheduling();

    /**
     * Process all events until none is pending
     *
     * This can busy-lock, if some task or system event always generates new
     * events when being processed. Most time it's called in unit tests to
     * process all pending events. Internally it just calls
     * Application::Reschedule( true ) until it fails.
     *
     * @see Application::Reschedule
     */
    static void       ProcessEventsToIdle();

    /**
     * Wakes up the scheduler
     *
     * This doesn't handle any events! It just ensures the Scheduler is run as
     * soon as possible by forcing the Scheduler timer to fire.
     *
     * Can be used for complex UpdateMinPeriod function, where the task is
     * actually active but not ready and we want to skip the Task::Start()
     * queue append for faster reaction.
     */
    static void       Wakeup();

    /// Control the deterministic mode.  In this mode, two subsequent runs of
    /// LibreOffice fire about the same amount idles.
    static void       SetDeterministicMode(bool bDeterministic);
    /// Return the current state of deterministic mode.
    static bool       GetDeterministicMode();

    // Makes sure that idles are not processed, until the guard is destroyed
    struct VCL_DLLPUBLIC IdlesLockGuard final
    {
        IdlesLockGuard();
        ~IdlesLockGuard();
    };
};

#endif // INCLUDED_VCL_SCHEDULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
