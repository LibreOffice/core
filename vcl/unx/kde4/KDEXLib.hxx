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

#pragma once

#include <memory>

#include <unx/saldisp.hxx>

#include <fixx11h.h>

#include <QtCore/QHash>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>

#include <unx/salinst.h>

class VCLKDEApplication;

class KDEXLib : public QObject, public SalXLib
{
    Q_OBJECT
    private:
        bool m_bStartupDone;
        std::unique_ptr<VCLKDEApplication> m_pApplication;
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
        int m_frameWidth;
        bool m_isGlibEventLoopType;
        bool m_allowKdeDialogs;
        bool blockIdleTimeout;

    private:
        void setupEventLoop();

    private Q_SLOTS:
        void socketNotifierActivated( int fd );
        void timeoutActivated();
        void userEventActivated();
        void startTimeoutTimer();
        void startUserEventTimer();
        bool processYield( bool bWait, bool bHandleAllCurrentEvents );
    Q_SIGNALS:
        void startTimeoutTimerSignal();
        void startUserEventTimerSignal();
        void processYieldSignal( bool bWait, bool bHandleAllCurrentEvents );
        css::uno::Reference< css::ui::dialogs::XFilePicker2 >
            createFilePickerSignal( const css::uno::Reference< css::uno::XComponentContext >& );
        int getFrameWidthSignal();

    public:
        KDEXLib();
        virtual ~KDEXLib() override;

        virtual void Init() override;
        virtual bool Yield( bool bWait, bool bHandleAllCurrentEvents ) override;
        virtual void Insert( int fd, void* data, YieldFunc pending, YieldFunc queued, YieldFunc handle ) override;
        virtual void Remove( int fd ) override;
        virtual void StartTimer( sal_uLong nMS ) override;
        virtual void StopTimer() override;
        virtual void Wakeup() override;
        virtual void PostUserEvent() override;

        void doStartup();
        bool allowKdeDialogs() { return m_allowKdeDialogs; }

    public Q_SLOTS:
        css::uno::Reference< css::ui::dialogs::XFilePicker2 >
            createFilePicker( const css::uno::Reference< css::uno::XComponentContext >& );
        int getFrameWidth();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
