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

#ifndef INCLUDED_VCL_TASK_HXX
#define INCLUDED_VCL_TASK_HXX

#include <vcl/dllapi.h>
#include <memory>

class Scheduler;
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

    ImplSchedulerData *mpSchedulerData; ///< Pointer to the element in scheduler list
    const sal_Char    *mpDebugName;     ///< Useful for debugging
    TaskPriority       mePriority;      ///< Task priority
    bool               mbActive;        ///< Currently in the scheduler

protected:
    static void StartTimer( sal_uInt64 nMS );

    const ImplSchedulerData* GetSchedulerData() const { return mpSchedulerData; }

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
    virtual ~Task() COVERITY_NOEXCEPT_FALSE;
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

#endif // INCLUDED_VCL_TASK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
