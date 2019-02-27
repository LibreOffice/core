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

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#include <unx/salunx.h>
#include <unx/saldisp.hxx>
#include <unx/saltimer.h>
#include <unx/salinst.h>

void SalXLib::StopTimer()
{
    m_aTimeout.tv_sec   = 0;
    m_aTimeout.tv_usec  = 0;
    m_nTimeoutMS        = 0;
}

void SalXLib::StartTimer( sal_uInt64 nMS )
{
    timeval Timeout (m_aTimeout); // previous timeout.
    gettimeofday (&m_aTimeout, nullptr);

    m_nTimeoutMS  = nMS;
    m_aTimeout    += m_nTimeoutMS;

    if ((Timeout > m_aTimeout) || (Timeout.tv_sec == 0))
    {
        // Wakeup from previous timeout (or stopped timer).
        Wakeup();
    }
}

SalTimer* X11SalInstance::CreateSalTimer()
{
    return new X11SalTimer( mpXLib );
}

X11SalTimer::~X11SalTimer()
{
}

void X11SalTimer::Stop()
{
    mpXLib->StopTimer();
}

void X11SalTimer::Start( sal_uInt64 nMS )
{
    mpXLib->StartTimer( nMS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
