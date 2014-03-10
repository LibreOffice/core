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
#include <qthread.h>
#include <qtimer.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>

class SocketEventsTester: public QObject
{
    Q_OBJECT
public:
    SocketEventsTester()
    {
        socket = 0;
        server = 0;
        dataSent = false;
        testResult = false;
        dataArrived = false;
    }
    ~SocketEventsTester()
    {
        delete socket;
        delete server;
    }
    bool init()
    {
        bool ret = false;
        server = new QTcpServer();
        socket = new QTcpSocket();
        connect(server, SIGNAL(newConnection()), this, SLOT(sendHello()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(sendAck()), Qt::DirectConnection);
        if((ret = server->listen(QHostAddress::LocalHost, 0))) {
            socket->connectToHost(server->serverAddress(), server->serverPort());
            socket->waitForConnected();
        }
        return ret;
    }

    QTcpSocket *socket;
    QTcpServer *server;
    bool dataSent;
    bool testResult;
    bool dataArrived;
public slots:
    void sendAck()
    {
        dataArrived = true;
    }
    void sendHello()
    {
        char data[10] ="HELLO";
        qint64 size = sizeof(data);

        QTcpSocket *serverSocket = server->nextPendingConnection();
        serverSocket->write(data, size);
        dataSent = serverSocket->waitForBytesWritten(-1);
        QEventLoop loop;
        //allow the TCP/IP stack time to loopback the data, so our socket is ready to read
        QTimer::singleShot(200, &loop, SLOT(quit()));
        loop.exec(QEventLoop::ExcludeSocketNotifiers);
        testResult = dataArrived;
        //check the deferred event is processed
        QTimer::singleShot(200, &loop, SLOT(quit()));
        loop.exec();
        serverSocket->close();
        QThread::currentThread()->exit(0);
    }
};

class SocketTestThread : public QThread
{
    Q_OBJECT
public:
    SocketTestThread():QThread(0),testResult(false){};
    void run()
    {
        SocketEventsTester *tester = new SocketEventsTester();
        if (tester->init())
            exec();
        dataSent = tester->dataSent;
        testResult = tester->testResult;
        dataArrived = tester->dataArrived;
        delete tester;
    }
    bool dataSent;
    bool testResult;
    bool dataArrived;
};

#define QVERIFY(a) \
    if (!a) return 1;

static int tst_processEventsExcludeSocket()
{
    SocketTestThread thread;
    thread.start();
    QVERIFY(thread.wait());
    QVERIFY(thread.dataSent);
    QVERIFY(!thread.testResult);
    QVERIFY(thread.dataArrived);
    return 0;
}

