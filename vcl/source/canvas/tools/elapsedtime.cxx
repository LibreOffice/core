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

#include <sal/config.h>

#include <vcl_canvas/elapsedtime.hxx>

#include <tools/time.hxx>
#include <utility>

namespace vcl_canvas::tools {

double ElapsedTime::getSystemTime()
{
    return ::tools::Time::GetMonotonicTicks() / 1.0E6;
}

ElapsedTime::ElapsedTime()
    : m_pTimeBase(),
      m_fLastQueriedTime( 0.0 ),
      m_fStartTime( getSystemTime() ),
      m_fFrozenTime( 0.0 ),
      m_bInPauseMode( false ),
      m_bInHoldMode( false )
{
}

ElapsedTime::ElapsedTime(
    std::shared_ptr<ElapsedTime> pTimeBase )
    : m_pTimeBase(std::move( pTimeBase )),
      m_fLastQueriedTime( 0.0 ),
      m_fStartTime( getCurrentTime() ),
      m_fFrozenTime( 0.0 ),
      m_bInPauseMode( false ),
      m_bInHoldMode( false )
{
}

void ElapsedTime::reset()
{
    m_fLastQueriedTime = 0.0;
    m_fStartTime = getCurrentTime();
    m_fFrozenTime = 0.0;
    m_bInPauseMode = false;
    m_bInHoldMode = false;
}

void ElapsedTime::adjustTimer( double fOffset )
{
    // to make getElapsedTime() become _larger_, have to reduce
    // m_fStartTime.
    m_fStartTime -= fOffset;

    // also adjust frozen time, this method must _always_ affect the
    // value returned by getElapsedTime()!
    if (m_bInHoldMode || m_bInPauseMode)
        m_fFrozenTime += fOffset;
}

double ElapsedTime::getCurrentTime() const
{
    return m_pTimeBase == nullptr ? getSystemTime() : m_pTimeBase->getElapsedTimeImpl();
}

double ElapsedTime::getElapsedTime() const
{
    m_fLastQueriedTime = getElapsedTimeImpl();
    return m_fLastQueriedTime;
}

double ElapsedTime::getElapsedTimeImpl() const
{
    if (m_bInHoldMode || m_bInPauseMode)
        return m_fFrozenTime;

    return getCurrentTime() - m_fStartTime;
}

void ElapsedTime::pauseTimer()
{
    m_fFrozenTime = getElapsedTimeImpl();
    m_bInPauseMode = true;
}

void ElapsedTime::continueTimer()
{
    m_bInPauseMode = false;

    // stop pausing, time runs again. Note that
    // getElapsedTimeImpl() honors hold mode, i.e. a
    // continueTimer() in hold mode will preserve the latter
    const double fPauseDuration( getElapsedTimeImpl() - m_fFrozenTime );

    // adjust start time, such that subsequent getElapsedTime() calls
    // will virtually start from m_fFrozenTime.
    m_fStartTime += fPauseDuration;
}

void ElapsedTime::holdTimer()
{
    // when called during hold mode (e.g. more than once per time
    // object), the original hold time will be maintained.
    m_fFrozenTime = getElapsedTimeImpl();
    m_bInHoldMode = true;
}

void ElapsedTime::releaseTimer()
{
    m_bInHoldMode = false;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
