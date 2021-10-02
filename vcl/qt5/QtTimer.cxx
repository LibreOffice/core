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

#include <QtTimer.hxx>
#include <QtTimer.moc>

#include <QtInstance.hxx>

#include <QtWidgets/QApplication>
#include <QtCore/QThread>

#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include <svdata.hxx>

QtTimer::QtTimer()
{
    m_aTimer.setSingleShot(true);
    m_aTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_aTimer, SIGNAL(timeout()), this, SLOT(timeoutActivated()));
    connect(this, SIGNAL(startTimerSignal(int)), this, SLOT(startTimer(int)));
    connect(this, SIGNAL(stopTimerSignal()), this, SLOT(stopTimer()));
}

void QtTimer::timeoutActivated()
{
    SolarMutexGuard aGuard;
#ifdef EMSCRIPTEN
    const ImplSVData* pSVData = ImplGetSVData();
    assert(pSVData->mpDefInst);
    bool ts = static_cast<QtInstance*>(pSVData->mpDefInst)->DispatchUserEvents(true);
#endif
    CallCallback();
}

void QtTimer::startTimer(int nMS) { m_aTimer.start(nMS); }

void QtTimer::Start(sal_uInt64 nMS) { Q_EMIT startTimerSignal(nMS); }

void QtTimer::stopTimer() { m_aTimer.stop(); }

void QtTimer::Stop() { Q_EMIT stopTimerSignal(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
