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
#include <vcl/scheduler.hxx>

class VCL_DLLPUBLIC Timer : public Scheduler
{
protected:
    Link<Timer *, void> maTimeoutHdl;          // Callback Link
    sal_uInt64      mnTimeout;
    bool            mbAuto;

    virtual void SetDeletionFlags() override;

    virtual bool ReadyForSchedule( const sal_uInt64 nTime, const bool bIdle ) const override;
    virtual void UpdateMinPeriod( const sal_uInt64 nTime, sal_uInt64 &nMinPeriod ) const override;

public:
    Timer( const sal_Char *pDebugName = nullptr );
    Timer( const Timer& rTimer );

    /// Make it possible to associate a callback with this timer handler
    /// of course, you can also sub-class and override 'Invoke'
    void            SetTimeoutHdl( const Link<Timer *, void>& rLink ) { maTimeoutHdl = rLink; }
    const Link<Timer *, void>& GetTimeoutHdl() const { return maTimeoutHdl; }
    void            SetTimeout( sal_uInt64 nTimeoutMs );
    sal_uInt64      GetTimeout() const { return mnTimeout; }
    virtual void    Invoke() override;
    void            Timeout() { Invoke(); }
    Timer&          operator=( const Timer& rTimer );
    virtual void    Start() override;
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
