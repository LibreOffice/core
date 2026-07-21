/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <canvas/elapsedtime.hxx>

#include <tools/time.hxx>
#include <utility>

namespace canvastools {

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

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
