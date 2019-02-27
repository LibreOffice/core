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

#ifndef INCLUDED_VCL_INC_OSX_SALTIMER_H
#define INCLUDED_VCL_INC_OSX_SALTIMER_H

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

#include <saltimer.hxx>

/**
 * if NO == bAtStart, then it has to be run in the main thread,
 * e.g. via performSelectorOnMainThread!
 **/
void ImplNSAppPostEvent( short nEventId, BOOL bAtStart, int nUserData = 0 );

class ReleasePoolHolder
{
    NSAutoreleasePool* mpPool;

public:
    ReleasePoolHolder() : mpPool( [[NSAutoreleasePool alloc] init] ) {}
    ~ReleasePoolHolder() { [mpPool release]; }
};

class AquaSalTimer final : public SalTimer, protected VersionedEvent
{
    NSTimer    *m_pRunningTimer;
    bool        m_bDirectTimeout;    ///< timeout can be processed directly

    void queueDispatchTimerEvent( bool bAtStart );
    void callTimerCallback();

public:
    AquaSalTimer();
    virtual ~AquaSalTimer() override;

    void Start( sal_uInt64 nMS ) override;
    void Stop() override;

    void handleStartTimerEvent( NSEvent* pEvent );
    bool handleDispatchTimerEvent( NSEvent* pEvent );
    void handleTimerElapsed();
    void handleWindowShouldClose();

    bool IsTimerElapsed() const;
    inline bool IsDirectTimeout() const;
};

inline bool AquaSalTimer::IsDirectTimeout() const
{
    return m_bDirectTimeout;
}

#endif // INCLUDED_VCL_INC_OSX_SALTIMER_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
