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

class Task;
struct TaskImpl;
struct ImplSchedulerContext;
struct ImplSchedulerData;

class VCL_DLLPUBLIC Scheduler final
{
    friend class Task;
    Scheduler() = delete;

    static inline bool HasPendingTasks( const ImplSchedulerContext &rSchedCtx,
                                        const sal_uInt64 nTime );

    static inline void UpdateMinPeriod( ImplSchedulerData *pSchedulerData,
                                        sal_uInt64 nTime, sal_uInt64 &nMinPeriod );

    static void ImplStartTimer ( sal_uInt64 nMS, bool bForce, sal_uInt64 nTime );

public:
    static constexpr sal_uInt64 ImmediateTimeoutMs = 1;
    static constexpr sal_uInt64 InfiniteTimeoutMs = 1000 * 60 * 60 * 24; // 1 day

    static void       ImplDeInitScheduler();

    /// Process one pending Timer with highhest priority
    static void       CallbackTaskScheduling();
    /// Are there any pending tasks to process?
    static bool       HasPendingTasks();
    /// Process one pending task ahead of time with highest priority.
    static bool       ProcessTaskScheduling();
    /// Process all events until we are idle
    static void       ProcessEventsToIdle();
    /**
     * Process events until the parameter turns true,
     * allows processing until a specific event has been processed
     */
    static void       ProcessEventsToSignal(bool& bSignal);

    /// Control the deterministic mode.  In this mode, two subsequent runs of
    /// LibreOffice fire about the same amount idles.
    static void       SetDeterministicMode(bool bDeterministic);
    /// Return the current state of deterministic mode.
    static bool       GetDeterministicMode();
};

#endif // INCLUDED_VCL_SCHEDULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
