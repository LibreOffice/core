/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#pragma once

#include <saldisp.hxx>

#include <fixx11h.h>

#include <qhash.h>
#include <qsocketnotifier.h>
#include <qtimer.h>

class VCLKDEApplication;

class KDEXLib : public QObject, public SalXLib
{
    Q_OBJECT
    private:
        bool m_bStartupDone;
        VCLKDEApplication* m_pApplication;
        char** m_pFreeCmdLineArgs;
        char** m_pAppCmdLineArgs;
        int m_nFakeCmdLineArgs;
        struct SocketData
            {
            void* data;
            YieldFunc pending;
            YieldFunc queued;
            YieldFunc handle;
            QSocketNotifier* notifier;
            };
        QHash< int, SocketData > socketData; // key is fd
        QTimer timeoutTimer;
        QTimer userEventTimer;
        enum { LibreOfficeEventLoop, GlibEventLoop, QtUnixEventLoop } eventLoopType;

    private:
        void setupEventLoop();

    private slots:
        void socketNotifierActivated( int fd );
        void timeoutActivated();
        void userEventActivated();
        void startTimeoutTimer();
        void startUserEventTimer();
        bool processYield( bool bWait, bool bHandleAllCurrentEvents );
    signals:
        void startTimeoutTimerSignal();
        void startUserEventTimerSignal();
        void processYieldSignal( bool bWait, bool bHandleAllCurrentEvents );

    public:
        KDEXLib();
        virtual ~KDEXLib();

        virtual void Init();
        virtual void Yield( bool bWait, bool bHandleAllCurrentEvents );
        virtual void Insert( int fd, void* data, YieldFunc pending, YieldFunc queued, YieldFunc handle );
        virtual void Remove( int fd );
        virtual void StartTimer( sal_uLong nMS );
        virtual void StopTimer();
        virtual void Wakeup();
        virtual void PostUserEvent();

        void doStartup();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
