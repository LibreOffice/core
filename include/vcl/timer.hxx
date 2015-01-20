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

#ifndef INCLUDED_VCL_TIMER_HXX
#define INCLUDED_VCL_TIMER_HXX

#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>

struct ImplTimerData;
struct ImplSVData;

/// Base-class for timers - usually a simple, one-shot timeout
class VCL_DLLPUBLIC Timer
{
protected:
    ImplTimerData*  mpTimerData;
    sal_uLong       mnTimeout;
    bool            mbActive;
    bool            mbAuto;
    bool            mbIdle;
    Link            maTimeoutHdl;

    friend struct ImplTimerData;

public:
                    Timer();
                    Timer( const Timer& rTimer );
    virtual         ~Timer();

    virtual void    Timeout();

    void            Start();
    void            Stop();

    /// set the timeout in milliseconds
    void            SetTimeout( sal_uLong nTimeoutMs );
    sal_uLong       GetTimeout() const { return mnTimeout; }
    bool            IsActive() const { return mbActive; }

    /// Make it possible to associate a callback with this timeout
    void            SetTimeoutHdl( const Link& rLink ) { maTimeoutHdl = rLink; }
    const Link&     GetTimeoutHdl() const { return maTimeoutHdl; }

    Timer&          operator=( const Timer& rTimer );

    static void ImplDeInitTimer();
    static void ImplTimerCallbackProc();

    /// Process all pending idle tasks ahead of time in priority order.
    static void ProcessAllIdleHandlers();
};

/// An auto-timer is a multi-shot timer re-emitting itself at
/// interval until destroyed.
class VCL_DLLPUBLIC AutoTimer : public Timer
{
public:
                    AutoTimer();
                    AutoTimer( const AutoTimer& rTimer );

    AutoTimer&      operator=( const AutoTimer& rTimer );
};

enum IdlePriority {
    VCL_IDLE_PRIORITY_HIGHEST, // -> 0ms
    VCL_IDLE_PRIORITY_HIGH,    // -> 1ms
    VCL_IDLE_PRIORITY_REPAINT, // -> 30ms
    VCL_IDLE_PRIORITY_RESIZE,  // -> 50ms
    VCL_IDLE_PRIORITY_MEDIUM,  // -> 50ms
    VCL_IDLE_PRIORITY_LOW,     // -> 100ms
    VCL_IDLE_PRIORITY_LOWER,   // -> 200ms
    VCL_IDLE_PRIORITY_LOWEST   // -> 400ms
};


// To port from Timer -> Idle switch class name,
// s/Timeout/DoIdle/ etc. and select priority
class VCL_DLLPUBLIC Idle : public Timer
{
 public:
    Idle();
    Idle( IdlePriority ePriority );
    virtual ~Idle();

    void SetPriority( IdlePriority ePriority );

    /// Make it possible to associate a callback with this idle handler
    /// of course, you can also sub-class and override 'DoIdle'
    void            SetIdleHdl( const Link& rLink ) { SetTimeoutHdl( rLink ); }
    const Link&     GetIdleHdl() const              { return GetTimeoutHdl(); }

    void            Start() { Timer::Start(); }
    void            Stop()  { Timer::Stop();  }

    void    DoIdle();

    virtual void    Timeout() SAL_OVERRIDE { DoIdle(); }
};

#endif // INCLUDED_VCL_TIMER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
