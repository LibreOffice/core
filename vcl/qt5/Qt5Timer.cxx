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

#include "Qt5Timer.hxx"
#include <Qt5Timer.moc>

#include <QtWidgets/QApplication>
#include <QtCore/QThread>

Qt5Timer::Qt5Timer()
{
    m_aTimer.setSingleShot(true);
    // run the timer itself in the main / creator thread
    connect(&m_aTimer, SIGNAL(timeout()), this, SLOT(timeoutActivated()), Qt::QueuedConnection);
    // QTimer::start() can be called only in its creator thread
    connect(this, SIGNAL(startTimerSignal()), this, SLOT(startTimer()), Qt::QueuedConnection);
}

Qt5Timer::~Qt5Timer() {}

void Qt5Timer::timeoutActivated() { CallCallback(); }

void Qt5Timer::startTimer() { m_aTimer.start(); }

void Qt5Timer::Start(sal_uIntPtr nMS)
{
    m_aTimer.setInterval(nMS);
    if (qApp->thread() == QThread::currentThread())
        startTimer();
    else
        Q_EMIT startTimerSignal();
}

void Qt5Timer::Stop() { m_aTimer.stop(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
