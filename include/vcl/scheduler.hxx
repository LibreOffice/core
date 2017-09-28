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

class SchedulerGuard;
class Task;
struct TaskImpl;
struct ImplSchedulerContext;
struct ImplSchedulerData;

class VCL_DLLPUBLIC Scheduler final
{
    friend class SchedulerGuard;
    friend class Task;
    Scheduler() SAL_DELETED_FUNCTION;

    static inline void UpdateSystemTimer( ImplSchedulerContext &rSchedCtx,
                                          sal_uInt64 nMinPeriod,
                                          bool bForce, sal_uInt64 nTime );

    static void ImplStartTimer ( sal_uInt64 nMS, bool bForce, sal_uInt64 nTime );

    static void Lock( sal_uInt32 nLockCount = 1 );
    static sal_uInt32 Unlock( bool bUnlockAll = false );

public:
    static constexpr sal_uInt64 ImmediateTimeoutMs = 0;
    static constexpr sal_uInt64 InfiniteTimeoutMs  = SAL_MAX_UINT64;

    static void       ImplDeInitScheduler();

    /// Process one pending Timer with highhest priority
    static void       CallbackTaskScheduling();
    /// Process one pending task ahead of time with highest priority.
    static bool       ProcessTaskScheduling();
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

    /// Control the deterministic mode.  In this mode, two subsequent runs of
    /// LibreOffice fire about the same amount idles.
    static void       SetDeterministicMode(bool bDeterministic);
    /// Return the current state of deterministic mode.
    static bool       GetDeterministicMode();
};

#endif // INCLUDED_VCL_SCHEDULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
