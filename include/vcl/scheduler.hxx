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

struct ImplSchedulerData;
struct ImplSVData;

enum class SchedulerPriority {
    HIGHEST,       ///< These events should run very fast!
    DEFAULT,       ///< Default priority used, e.g. the default timer priority
    HIGH_IDLE,     ///< Important idle events to be run before processing drawing events
    RESIZE,        ///< Resize runs before repaint, so we won't paint twice
    REPAINT,       ///< All repaint events should go in here
    POST_PAINT,    ///< Everything running directly after painting
    DEFAULT_IDLE,  ///< Default idle priority
    LOWEST         ///< Low, very idle cleanup tasks
};

enum class IdleRunPolicy {
    IDLE_VIA_TIMER,  ///< Idles are scheduled via immediate timers (ImmediateTimeoutMs)
    IDLE_VIA_LOOP    ///< Return indicates processed events, so they are processed in a loop
};

class VCL_DLLPUBLIC Scheduler
{
    friend struct ImplSchedulerData;

private:
    static inline void UpdateMinPeriod( ImplSchedulerData *pSchedulerData,
                                        const sal_uInt64 nTime, sal_uInt64 &nMinPeriod );

    static inline bool HasPendingEvents( const ImplSVData* pSVData, const sal_uInt64 nTime );

protected:
    ImplSchedulerData*  mpSchedulerData;    /// Pointer to element in scheduler list
    const sal_Char     *mpDebugName;        /// Useful for debugging
    SchedulerPriority   mePriority;         /// Scheduler priority

    static const SAL_CONSTEXPR sal_uInt64 ImmediateTimeoutMs = 0;
    static const SAL_CONSTEXPR sal_uInt64 InfiniteTimeoutMs  = SAL_MAX_UINT64;

    static void ImplStartTimer(sal_uInt64 nMS, bool bForce = false);

    virtual void SetDeletionFlags();

    virtual bool ReadyForSchedule( const sal_uInt64 nTime ) const = 0;
    virtual void UpdateMinPeriod( const sal_uInt64 nTime, sal_uInt64 &nMinPeriod ) const = 0;

public:
    Scheduler( const sal_Char *pDebugName );
    Scheduler( const Scheduler& rScheduler );
    virtual ~Scheduler();

    void SetPriority(SchedulerPriority ePriority) { mePriority = ePriority; }
    SchedulerPriority GetPriority() const { return mePriority; }

    void            SetDebugName( const sal_Char *pDebugName ) { mpDebugName = pDebugName; }
    const char     *GetDebugName() { return mpDebugName; }

    // Call handler
    virtual void    Invoke() = 0;

    virtual void    Start();
    void            Stop();

    inline bool     IsActive() const;

    Scheduler&      operator=( const Scheduler& rScheduler );
    static void     ImplDeInitScheduler();

    /// Process one pending Timer with highhest priority
    static void       CallbackTaskScheduling();
    /// Process one pending task ahead of time with highest priority.
    static bool       ProcessTaskScheduling( IdleRunPolicy eIdleRunPolicy
                          = IdleRunPolicy::IDLE_VIA_TIMER );
    /// Process all events until we are idle
    static void       ProcessAllPendingEvents();
    /// Are there any pending tasks in the LO task queue?
    static bool       HasPendingEvents();

    /// Control the deterministic mode.  In this mode, two subsequent runs of
    /// LibreOffice fire about the same amount idles.
    static void SetDeterministicMode(bool bDeterministic);
    /// Return the current state of deterministic mode.
    static bool GetDeterministicMode();
};

inline bool Scheduler::IsActive() const
{
    return nullptr != mpSchedulerData;
}

#endif // INCLUDED_VCL_SCHEDULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
