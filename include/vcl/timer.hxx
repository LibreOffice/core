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

enum IdlePriority {
        VCL_IDLE_PRIORITY_HIGHEST, //   -> 0ms
        VCL_IDLE_PRIORITY_HIGH, //      -> 1ms
        VCL_IDLE_PRIORITY_REPAINT, //    -> 30ms
        VCL_IDLE_PRIORITY_RESIZE,  //    -> 50ms
        VCL_IDLE_PRIORITY_MEDIUM, //     -> 50ms
        VCL_IDLE_PRIORITY_LOW, //        -> 100ms
        VCL_IDLE_PRIORITY_LOWER, //      -> 200ms
        VCL_IDLE_PRIORITY_LOWEST //     -> 400ms
};

/// Base-class for timers - usually a simple, one-shot timeout
class VCL_DLLPUBLIC Timer
{
protected:
    ImplTimerData*  mpTimerData;
    sal_Int32       mnDefaultPriority;
    sal_Int32       mnPriority;
    sal_uLong       mnTimeout;
    bool            mbActive;
    bool            mbAuto;
    Link            maTimeoutHdl;

public:
                    Timer();
                    Timer( const Timer& rTimer );
    virtual         ~Timer();

    virtual void    Timeout();

    void            Start();
    void            Stop();

    /// set the timeout in milliseconds and the priority
    void            SetTimeout( sal_uLong nTimeoutMs, sal_Int32 nPriority = VCL_IDLE_PRIORITY_MEDIUM );
    void            SetPriority( const sal_Int32 nPriority ) { mnPriority = nPriority; }
    sal_Int32       GetPriority() const { return mnPriority; }
    sal_Int32       GetDefaultPriority() const { return mnDefaultPriority; }
    sal_uLong       GetTimeout() const { return mnTimeout; }
    bool            IsActive() const { return mbActive; }

    /// Make it possible to associate a callback with this timeout
    void            SetTimeoutHdl( const Link& rLink ) { maTimeoutHdl = rLink; }
    const Link&     GetTimeoutHdl() const { return maTimeoutHdl; }

    Timer&          operator=( const Timer& rTimer );

    static void ImplDeInitTimer();
    static void ImplTimerCallbackProc();
    sal_Int32 getIntFromEnum (IdlePriority e){
        switch (e) {
            case VCL_IDLE_PRIORITY_HIGHEST: return -40;
            case VCL_IDLE_PRIORITY_HIGH: return -30;
            case VCL_IDLE_PRIORITY_REPAINT: return -20;
            case VCL_IDLE_PRIORITY_RESIZE: return -10;
            case VCL_IDLE_PRIORITY_MEDIUM: return 0;
            case VCL_IDLE_PRIORITY_LOW: return 10;
            case VCL_IDLE_PRIORITY_LOWER: return 20;
            case VCL_IDLE_PRIORITY_LOWEST: return 30;
        }
    }
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

#endif // INCLUDED_VCL_TIMER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
