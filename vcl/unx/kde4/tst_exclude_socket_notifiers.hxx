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
 *
 * This code is based on the SocketEventsTester from the Qt4 test suite.
 */

#pragma once

#include <qcoreapplication.h>
#include <qeventloop.h>
#include <qsocketnotifier.h>
#include <unistd.h>

namespace
{

class TestExcludeSocketNotifiers
    : public QObject
{
    Q_OBJECT
    public:
        TestExcludeSocketNotifiers( const int* pipes );
        virtual ~TestExcludeSocketNotifiers();
        bool received;
    public slots:
        void slotReceived();
    private:
        const int* pipes;
};

TestExcludeSocketNotifiers::TestExcludeSocketNotifiers( const int* pipes )
    : received( false )
    , pipes( pipes )
{
}

TestExcludeSocketNotifiers::~TestExcludeSocketNotifiers()
{
    close( pipes[ 0 ] );
    close( pipes[ 1 ] );
}

void TestExcludeSocketNotifiers::slotReceived()
{
    received = true;
}

}

#define QVERIFY(a) \
    if (!a) return 1;

static int tst_processEventsExcludeSocket()
{
    int pipes[ 2 ];
    if( pipe( pipes ) < 0 )
        return 1;
    TestExcludeSocketNotifiers test( pipes );
    QSocketNotifier notifier( pipes[ 0 ], QSocketNotifier::Read );
    QObject::connect( &notifier, SIGNAL( activated( int )), &test, SLOT( slotReceived()));
    char dummy = 'a';
    if( 1 != write( pipes[ 1 ], &dummy, 1 ) )
        return 1;
    QEventLoop loop;
    loop.processEvents( QEventLoop::ExcludeSocketNotifiers );
    QVERIFY( !test.received );
    loop.processEvents();
    QVERIFY( test.received );
    return 0;
}
