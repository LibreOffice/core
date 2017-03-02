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

class VCL_DLLPUBLIC Scheduler
{
    friend class Task;
    Scheduler() = delete;

protected:
    static void ImplStartTimer ( sal_uInt64 nMS, bool bForce = false );

public:
    static constexpr sal_uInt64 ImmediateTimeoutMs = 1;
    static constexpr sal_uInt64 InfiniteTimeoutMs = 1000 * 60 * 60 * 24; // 1 day

    static void       ImplDeInitScheduler();

    /// Process one pending Timer with highhest priority
    static void       CallbackTaskScheduling( bool ignore );
    /// Calculate minimum timeout - and return its value.
    static sal_uInt64 CalculateMinimumTimeout( bool &bHasActiveIdles );
    /// Process one pending task ahead of time with highest priority.
    static bool       ProcessTaskScheduling( bool bIdle );
    /**
     * Process events until the parameter turns true,
     * allows processing until a specific event has been processed
     */
    static void       ProcessEventsToIdle();
    static void       ProcessEventsToSignal(bool& bSignal);

    /// Control the deterministic mode.  In this mode, two subsequent runs of
    /// LibreOffice fire about the same amount idles.
    static void       SetDeterministicMode(bool bDeterministic);
    /// Return the current state of deterministic mode.
    static bool       GetDeterministicMode();
};


struct ImplSchedulerData;

enum class TaskPriority
{
    HIGHEST      = 0,
    HIGH         = 1,
    RESIZE       = 2,
    REPAINT      = 3,
    MEDIUM       = 3,
    POST_PAINT   = 4,
    DEFAULT_IDLE = 5,
    LOW          = 6,
    LOWER        = 7,
    LOWEST       = 8
};

class VCL_DLLPUBLIC Task
{
    friend class Scheduler;
    friend struct ImplSchedulerData;

    ImplSchedulerData *mpSchedulerData; /// Pointer to the element in scheduler list
    const sal_Char    *mpDebugName;     /// Useful for debugging
    TaskPriority       mePriority;      /// Task priority
    bool               mbActive;        /// Currently in the scheduler

protected:
    static void StartTimer( sal_uInt64 nMS );

    inline const ImplSchedulerData* GetSchedulerData() const { return mpSchedulerData; }

    virtual void SetDeletionFlags();
    /// Is this item ready to be dispatched at nTimeNow
    virtual bool ReadyForSchedule( bool bIdle, sal_uInt64 nTimeNow ) const = 0;
    /// Schedule only when other timers and events are processed
    virtual bool IsIdle() const = 0;
    /**
     * Adjust nMinPeriod downwards if we want to be notified before
     * then, nTimeNow is the current time.
     */
    virtual sal_uInt64 UpdateMinPeriod( sal_uInt64 nMinPeriod, sal_uInt64 nTimeNow ) const = 0;

public:
    Task( const sal_Char *pDebugName );
    Task( const Task& rTask );
    virtual ~Task();
    Task& operator=( const Task& rTask );

    void            SetPriority(TaskPriority ePriority) { mePriority = ePriority; }
    TaskPriority    GetPriority() const { return mePriority; }

    void            SetDebugName( const sal_Char *pDebugName ) { mpDebugName = pDebugName; }
    const char     *GetDebugName() const { return mpDebugName; }

    // Call handler
    virtual void    Invoke() = 0;

    virtual void    Start();
    void            Stop();

    bool            IsActive() const { return mbActive; }
};

#endif // INCLUDED_VCL_SCHEDULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
