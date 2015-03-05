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
    Scheduler*          mpScheduler;      // Pointer to VCL Idle instance
    bool                mbDelete;    // Destroy this idle?
    bool                mbInScheduler;    // Idle handler currently processed?
    sal_uLong           mnUpdateTime;   // Last Update Time
    sal_uLong           mnUpdateStack;  // Update Stack on stack

    void Invoke();

    static ImplSchedulerData *GetMostImportantTask( bool bTimer );
};

enum class SchedulerPriority {
    HIGHEST   = 0,
    HIGH      = 1,
    REPAINT   = 2,
    RESIZE    = 3,
    MEDIUM    = 3,
    LOW       = 4,
    LOWER     = 5,
    LOWEST    = 6
};

class VCL_DLLPUBLIC Scheduler
{
protected:
    ImplSchedulerData*  mpSchedulerData;    // Pointer to element in idle list
    sal_Int32           miPriority;         // Idle priority ( maybe divergent to default)
    SchedulerPriority   meDefaultPriority;  // Default idle priority
    bool                mbActive;           // Currently in the scheduler

    friend struct ImplSchedulerData;
    virtual void SetDeletionFlags();
    virtual bool ReadyForSchedule( bool bTimer ) { return !bTimer; }
    virtual sal_uLong UpdateMinPeriod( sal_uLong nMinPeriod, sal_uLong nTime );

public:
    Scheduler();
    Scheduler( const Scheduler& rScheduler );
    virtual ~Scheduler();

    void SetPriority( SchedulerPriority ePriority );
    void SetSchedulingPriority( sal_Int32 iPriority );
    sal_Int32    GetPriority() const { return miPriority; }
    SchedulerPriority GetDefaultPriority() const { return meDefaultPriority; }

    // Call idle handler
    virtual void    Invoke() = 0;

    virtual void    Start();
    virtual void    Stop();

    bool            IsActive() const { return mbActive; }

    Scheduler&          operator=( const Scheduler& rScheduler );
    static void ImplDeInitScheduler();

    /// Process all pending idle tasks ahead of time in priority order.
    static void CallbackTaskScheduling( bool ignore );
    static void ProcessTaskScheduling( bool bTimer );
};

#endif // INCLUDED_VCL_SCHEDULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
