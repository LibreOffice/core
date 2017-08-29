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

#include <sal/main.h>
#include <osl/mutex.h>
#include <osl/conditn.h>
#include <osl/thread.h>

#include <cstdio>
#include <cassert>

#define BUFFER_SIZE 10

bool queue[9];
sal_uInt32 nItemCount = 0;

oslCondition fullOrEmpty;
oslMutex queueMutex;
oslThread producer, consumer;

void add();
void remove();

void produce(void*);
void consume(void*);

SAL_IMPLEMENT_MAIN()
{
    fprintf(stdout, "Producer/consumer problem - demonstrates mutex, condition variables and threads.\n");

    queueMutex = osl_createMutex();
    fullOrEmpty = osl_createCondition();

    producer = osl_createThread(produce, nullptr);
    consumer = osl_createThread(consume, nullptr);

    osl_joinWithThread(consumer);

    return 0;
}

void produce(void* /* pData */)
{
    osl_setThreadName("producer");

    while(true)
    {
        /* producer monitor - first acquire exclusive access to
           the queue, if the queue is full then wait till there
           is space made available. Once the queue is no longer
           full, then notify that this is the case.
         */
        osl_acquireMutex(queueMutex);

        while (nItemCount == BUFFER_SIZE-1)
            osl_waitCondition(fullOrEmpty, nullptr);

        fprintf(stdout, "produce()\n");

        add();

        osl_setCondition(fullOrEmpty);
        osl_releaseMutex(queueMutex);

        osl_yieldThread();
    }

    fprintf(stderr, "exit produce()!\n");
}

void consume(void* /* pData */)
{
    osl_setThreadName("consumer");

    while(true)
    {
        /* consumer monitor - first acquire exclusive access to the
           queue, if the queue is empty then wait till something is
           produced. Once the queue is no longer empty, then notify
           that this is the case.
         */
        osl_acquireMutex(queueMutex);

        while (nItemCount == 0)
            osl_waitCondition(fullOrEmpty, nullptr);

        fprintf(stdout, "consume()\n");

        remove();

        osl_setCondition(fullOrEmpty);
        osl_releaseMutex(queueMutex);

        osl_yieldThread();
    }

    fprintf(stderr, "exit consume()!\n");
}

void add()
{
    queue[nItemCount] = true;

    fprintf(stdout, "Adding to queue - item %d added.\n", nItemCount);

    nItemCount++;
    assert(nItemCount <= BUFFER_SIZE-1);
}

void remove()
{
    queue[nItemCount] = false;

    fprintf(stdout, "Removing from queue - item %d removed.\n", nItemCount);

    nItemCount--;
    assert(nItemCount >= 0);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
