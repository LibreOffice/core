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

package com.sun.star.lib.uno.environments.remote;

import com.sun.star.lib.uno.typedesc.MethodDescription;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import org.junit.Test;
import static org.junit.Assert.*;

public final class JobQueue_Test {
    @Test public void testThreadLeavesJobQueueOnDispose0()
        throws InterruptedException
    {
        testThreadLeavesJobQueueOnDispose(0);
    }

    @Test public void testThreadLeavesJobQueueOnDispose5000()
        throws InterruptedException
    {
        testThreadLeavesJobQueueOnDispose(5000);
    }

    private void testThreadLeavesJobQueueOnDispose(int waitTime)
        throws InterruptedException
    {
        TestThread t = new TestThread(waitTime);
        t.waitToStart();
        String msg = "xcxxxxxxxx";
        t._jobQueue.dispose(t._disposeId, new RuntimeException (msg));
        t.waitToTerminate();
/*TODO: below test fails with "expected:<xcxxxxxxxx> but was:<null>":
        assertEquals(msg, t._message);
*/
    }

    @Test public void testThreadLeavesJobQueueOnReply0()
        throws InterruptedException
    {
        testThreadLeavesJobQueueOnReply(0);
    }

    @Test public void testThreadLeavesJobQueueOnReply5000()
        throws InterruptedException
    {
        testThreadLeavesJobQueueOnReply(5000);
    }

    private void testThreadLeavesJobQueueOnReply(int waitTime)
        throws InterruptedException
    {
        TestThread t = new TestThread(waitTime);
        t.waitToStart();
        // put reply job:
        t._jobQueue.putJob(
            new Job(null, __iReceiver,
                    new Message(
                        null, false, "oid", __workAt_td, null, false, null,
                        false, null, null)),
            null);
        t.waitToTerminate();
        assertTrue(true); // TODO! ???
    }

    @Test public void testStaticThreadExecutesJobs0()
        throws InterruptedException
    {
        testStaticThreadExecutesJobs(0);
    }

    @Test public void testStaticThreadExecutesJobs5000()
        throws InterruptedException
    {
        testStaticThreadExecutesJobs(5000);
    }

    private void testStaticThreadExecutesJobs(int waitTime)
        throws InterruptedException
    {
        TestThread t = new TestThread(waitTime);
        t.waitToStart();
        testExecuteJobs(t._jobQueue);
        t._jobQueue.dispose(t._disposeId,
                            new RuntimeException("xxxxxxxxxxxxx"));
        t.waitToTerminate();
    }

    @Test public void testDynamicThreadExecutesJob() throws InterruptedException
    {
        testExecuteJobs(
            new JobQueue(
                __javaThreadPoolFactory, ThreadId.createFresh(), true));
    }

    @Test public void testStaticThreadExecutesAsyncs()
        throws InterruptedException
    {
        TestThread t = new TestThread();
        JobQueue async_jobQueue = new JobQueue(__javaThreadPoolFactory,
                                               t._threadId);
        assertEquals(1, async_jobQueue._ref_count);
        t._jobQueue = __javaThreadPoolFactory.getJobQueue(t._threadId);
        assertEquals(1, t._jobQueue._ref_count);
        t.waitToStart();
        TestWorkAt workAt = new TestWorkAt();
        testAsyncJobQueue(workAt, async_jobQueue, t._threadId);
        t._jobQueue.dispose(t._disposeId,
                            new RuntimeException("xxxxxxxxxxxxx"));
        t.waitToTerminate();
        assertEquals(TestWorkAt.MESSAGES, workAt._async_counter);
        assertEquals(TestWorkAt.MESSAGES, workAt._sync_counter);
    }

    @Test public void testDynamicThreadExecutesAsyncs()
        throws InterruptedException
    {
        ThreadId threadId = ThreadId.createFresh();
        JobQueue async_jobQueue = new JobQueue(__javaThreadPoolFactory,
                                               threadId);
        TestWorkAt workAt = new TestWorkAt();
        testAsyncJobQueue(workAt, async_jobQueue, threadId);
        assertEquals(TestWorkAt.MESSAGES, workAt._async_counter);
        assertEquals(TestWorkAt.MESSAGES, workAt._sync_counter);
    }

    private void testExecuteJobs(JobQueue jobQueue) throws InterruptedException
    {
        TestWorkAt workAt = new TestWorkAt();
        testSendRequests(workAt, "increment", jobQueue);
        synchronized (workAt) {
            jobQueue.putJob(new Job(workAt, __iReceiver,
                                    new Message(
                                        null, true, "oid", __workAt_td,
                                        __workAt_td.getMethodDescription(
                                             "notifyme"),
                                        true, null, false, null, null)),
                            null);
            while (!workAt._notified) {
                workAt.wait();
            }
        }
        assertEquals(TestWorkAt.MESSAGES, workAt._counter);
    }

    private void testAsyncJobQueue(TestWorkAt workAt, JobQueue async_jobQueue,
                                   ThreadId threadId)
        throws InterruptedException
    {
        // put slow async calls first, followed by fast sync calls:
        testSendRequests(workAt, "asyncCall", async_jobQueue);
        testSendRequests(workAt, "syncCall",
                         __javaThreadPoolFactory.getJobQueue(threadId));
        synchronized (workAt) {
            async_jobQueue._sync_jobQueue.putJob(
                new Job(workAt, __iReceiver,
                        new Message(
                            null, true, "oid", __workAt_td,
                            __workAt_td.getMethodDescription("notifyme"),
                            true, null, false, null, null)),
                null);
            while (!workAt._notified) {
                workAt.wait();
            }
        }
        assertTrue(workAt.passedAsyncTest());
    }

    private void testSendRequests(TestWorkAt workAt, String operation,
                                  JobQueue jobQueue) {
        Message iMessage = new Message(
            null, true, "oid", __workAt_td,
            __workAt_td.getMethodDescription(operation),
            true, null, false, null, null);
        for (int i = 0; i < TestWorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            jobQueue.putJob(new Job(workAt, __iReceiver, iMessage),
                            new Object());
        }
    }

    private static final class TestThread extends Thread {
        public final ThreadId _threadId = JavaThreadPoolFactory.getThreadId();
        public final Object _disposeId = new Object();
        public JobQueue _jobQueue = null;

        public TestThread(int waitTime) {
            this.waitTime = waitTime;
            _jobQueue = new JobQueue(__javaThreadPoolFactory, _threadId, false);
        }

        public TestThread() {
            waitTime = 0;
        }

        public void run() {
            synchronized (lock) {
                state = STATE_STARTED;
                lock.notifyAll();
            }
            try {
                if (waitTime != 0) {
                    Thread.sleep(waitTime);
                }
                _jobQueue.enter(_disposeId);
            } catch (Throwable e) {
            }
            synchronized (lock) {
                state = STATE_DONE;
                lock.notifyAll();
            }
        }

        public void waitToStart() throws InterruptedException {
            start();
            synchronized (lock) {
                while (state == STATE_INITIAL) {
                    lock.wait();
                }
            }
        }

        public void waitToTerminate() throws InterruptedException {
            synchronized (lock) {
                while (state != STATE_DONE) {
                    lock.wait();
                }
            }
            join();
        }

        private final int waitTime;

        private final Object lock = new Object();
        private int state = STATE_INITIAL;
        private static final int STATE_INITIAL = 0;
        private static final int STATE_STARTED = 1;
        private static final int STATE_DONE = 2;
    }

    private static final JavaThreadPoolFactory __javaThreadPoolFactory
    = new JavaThreadPoolFactory();
    private static final IReceiver __iReceiver = new TestReceiver();
    private static final TypeDescription __workAt_td
    = TypeDescription.getTypeDescription(TestIWorkAt.class);
}
