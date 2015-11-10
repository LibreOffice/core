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

struct ImplSVData;
class Scheduler;
struct ImplSchedulerData
{
    ImplSchedulerData*  mpNext;      // Pointer to the next element in list
    Scheduler*          mpScheduler;      // Pointer to VCL Scheduler instance
    bool                mbDelete;    // Destroy this scheduler?
    bool                mbInScheduler;    // Scheduler currently processed?
    sal_uInt64          mnUpdateTime;   // Last Update Time
    sal_uInt32          mnUpdateStack;  // Update Stack

    void Invoke();

    static ImplSchedulerData *GetMostImportantTask( bool bTimer );
};

enum class SchedulerPriority {
    HIGHEST   = 0,
    HIGH      = 1,
    RESIZE    = 2,
    REPAINT   = 3,
    MEDIUM    = 3,
    LOW       = 4,
    LOWER     = 5,
    LOWEST    = 6
};

class VCL_DLLPUBLIC Scheduler
{
private:
    static void InitSystemTimer(ImplSVData* pSVData);

protected:
    ImplSchedulerData*  mpSchedulerData;    /// Pointer to element in scheduler list
    const sal_Char     *mpDebugName;        /// Useful for debugging
    SchedulerPriority   mePriority;         /// Scheduler priority
    bool                mbActive;           /// Currently in the scheduler

    // These should be constexpr static, when supported.
    static const sal_uInt64 ImmediateTimeoutMs = 1;
    static const sal_uInt64 MaximumTimeoutMs = SAL_MAX_UINT64;

    static void ImplStartTimer(sal_uInt64 nMS, bool bForce = false);

    friend struct ImplSchedulerData;
    virtual void SetDeletionFlags();
    virtual bool ReadyForSchedule( bool bTimer ) const = 0;
    virtual sal_uInt64 UpdateMinPeriod( sal_uInt64 nMinPeriod, sal_uInt64 nTime ) const = 0;

public:
    Scheduler( const sal_Char *pDebugName = nullptr );
    Scheduler( const Scheduler& rScheduler );
    virtual ~Scheduler();

    void SetPriority(SchedulerPriority ePriority) { mePriority = ePriority; }
    SchedulerPriority GetPriority() const { return mePriority; }

    void            SetDebugName( const sal_Char *pDebugName ) { mpDebugName = pDebugName; }

    // Call handler
    virtual void    Invoke() = 0;

    virtual void    Start();
    void            Stop();

    bool            IsActive() const { return mbActive; }

    Scheduler&      operator=( const Scheduler& rScheduler );
    static void ImplDeInitScheduler();

    // Process one pending Timer with highhest priority
    static void CallbackTaskScheduling( bool ignore );
    /// Process one pending task ahead of time with highhest priority.
    static void ProcessTaskScheduling( bool bTimer );
};

#endif // INCLUDED_VCL_SCHEDULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
