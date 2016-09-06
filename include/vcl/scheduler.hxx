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

enum class SchedulerPriority {
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

class VCL_DLLPUBLIC Scheduler
{
    friend struct ImplSchedulerData;

private:
    static inline void UpdateMinPeriod( ImplSchedulerData *pSchedulerData,
                                        const sal_uInt64 nTime, sal_uInt64 &nMinPeriod );

protected:
    ImplSchedulerData*  mpSchedulerData;    /// Pointer to element in scheduler list
    const sal_Char     *mpDebugName;        /// Useful for debugging
    SchedulerPriority   mePriority;         /// Scheduler priority

    static const SAL_CONSTEXPR sal_uInt64 ImmediateTimeoutMs = 1;
    static const SAL_CONSTEXPR sal_uInt64 InfiniteTimeoutMs  = SAL_MAX_UINT64;

    static void ImplStartTimer(sal_uInt64 nMS, bool bForce = false);

    virtual void SetDeletionFlags();

    virtual bool ReadyForSchedule( const sal_uInt64 nTime, const bool bIdle ) const = 0;
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
    static void ImplDeInitScheduler();

    /// Process one pending Timer with highhest priority
    static void CallbackTaskScheduling( bool bIdle );
    /// Process one pending task ahead of time with highest priority.
    static bool       ProcessTaskScheduling( bool bIdle );
    /// Process all events until we are idle
    static void       ProcessEventsToIdle();

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
