/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ThreadPool_Test.java,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:14:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package com.sun.star.lib.uno.environments.remote;

import com.sun.star.lib.uno.typedesc.TypeDescription;
import complexlib.ComplexTestCase;

public class ThreadPool_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "testDispose",
                              "testThreadAsync",
                              "testDynamicThreadSync",
                              "testStaticThreadSync",
                              "testDynamicThreadAsyncSyncOrder",
                              "testStaticThreadAsyncSyncOrder",
                              "testStress",
                              "testAsyncSync" };
    }

    public void testDispose() throws InterruptedException {
        IThreadPool iThreadPool = ThreadPoolManager.create();
        TestThread testThread = new TestThread(iThreadPool);

        ThreadId threadId = null;

        // start the test thread
        synchronized(testThread) {
            testThread.start();

            testThread.wait();

            threadId = testThread._threadId;

            // let the thread attach and enter the threadpool
            testThread.notifyAll();
        }

        String message = "blabla";

        // terminate the test thread
        synchronized(testThread) {
            // put reply job
            iThreadPool.dispose(new RuntimeException(message));

            testThread.wait();
        }

        testThread.join();

        assure("", testThread._message.equals(message));
    }

    public void testThreadAsync() throws InterruptedException {
        TestWorkAt workAt = new TestWorkAt();

        ThreadId threadId = ThreadId.createFresh();

        // queue asyncs
        for(int i = 0; i < TestWorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, false, threadId, "increment");
        }

        synchronized(workAt) {
            putJob(workAt, false, threadId, "notifyme");

            while(!workAt._notified)
                workAt.wait();
        }

        assure("", workAt._counter == TestWorkAt.MESSAGES);
    }

    public void testDynamicThreadSync() throws InterruptedException {
        TestWorkAt workAt = new TestWorkAt();

        ThreadId threadId = ThreadId.createFresh();

        // queue asyncs
        for(int i = 0; i < TestWorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, true, threadId, "increment");
        }

        synchronized(workAt) {
            putJob(workAt, true, threadId, "notifyme");

            while(!workAt._notified)
                workAt.wait();
        }

        assure("", workAt._counter == TestWorkAt.MESSAGES);
    }

    public void testStaticThreadSync() throws InterruptedException {
        TestWorkAt workAt = new TestWorkAt();

        TestThread testThread = new TestThread();

        ThreadId threadId = null;

        // start the test thread
        synchronized(testThread) {
            testThread.start();

            testThread.wait();

            threadId = testThread._threadId;

            // let the thread attach and enter the threadpool
            testThread.notifyAll();
        }

        // queue syncs
        for(int i = 0; i < TestWorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, true, threadId, "increment");
        }

        // terminate the test thread
        synchronized(testThread) {
            // put reply job
            putJob(workAt, true, threadId, null);

            testThread.wait();
        }

        testThread.join();

        assure("", workAt._counter == TestWorkAt.MESSAGES);
    }

    public void testDynamicThreadAsyncSyncOrder() throws InterruptedException {
        TestWorkAt workAt = new TestWorkAt();

        ThreadId threadId = ThreadId.createFresh();

        // queue asyncs
        for(int i = 0; i < TestWorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, false, threadId, "asyncCall");
        }

        // queue syncs
        for(int i = 0; i < TestWorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, true, threadId, "syncCall");
        }

        synchronized(workAt) {
            putJob(workAt, true, threadId, "notifyme");

            while(!workAt._notified)
                workAt.wait();
        }

        assure("", workAt.passedAsyncTest());
    }

    public void testStaticThreadAsyncSyncOrder() throws InterruptedException {
        TestWorkAt workAt = new TestWorkAt();

        TestThread testThread = new TestThread();

        // start the test thread
        synchronized(testThread) {
            testThread.start();

            testThread.wait();
        }

        ThreadId threadId = testThread._threadId;

        // queue asyncs
        for(int i = 0; i < TestWorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, false, threadId, "asyncCall");
        }

        // let the thread attach and enter the threadpool
        synchronized(testThread) {
            testThread.notifyAll();
        }

        // queue syncs
        for(int i = 0; i < TestWorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, true, threadId, "syncCall");
        }

        // terminate the test thread
        synchronized(testThread) {
            // put reply job
            putJob(workAt, true, threadId, null);

            testThread.wait();
        }

        testThread.join();

        assure("", workAt.passedAsyncTest());
    }

    public void testStress() throws InterruptedException {
        TestWorkAt workAt = new TestWorkAt();
        for (int i = 0; i < TestWorkAt.MESSAGES; ++i) {
            Thread.yield(); // force scheduling
            ThreadId threadID = ThreadId.createFresh();
            putJob(workAt, true, threadID, "increment");
            putJob(workAt, false, threadID, "increment");
        }
        synchronized (workAt) {
            while (workAt._counter < 2 * TestWorkAt.MESSAGES) {
                workAt.wait();
            }
        }

        abstract class Stress extends Thread {
            public Stress(int count) {
                this.count = count;
            }

            public void run() {
                try {
                    for (int i = 0; i < count; ++i) {
                        runTest();
                    }
                } catch (Throwable e) {
                    e.printStackTrace(System.err);
                }
            }

            protected abstract void runTest() throws InterruptedException;

            private final int count;
        };

        Stress stress1 = new Stress(50) {
                protected void runTest() throws InterruptedException {
                    testThreadAsync();
                }
            };
        stress1.start();

        Stress stress2 = new Stress(50) {
                protected void runTest() throws InterruptedException {
                    testDynamicThreadSync();
                }
            };
        stress2.start();

        Stress stress3 = new Stress(50) {
                protected void runTest() throws InterruptedException {
                    testStaticThreadSync();
                }
            };
        stress3.start();

        Stress stress4 = new Stress(50) {
                protected void runTest() throws InterruptedException {
                    testDynamicThreadAsyncSyncOrder();
                }
            };
        stress4.start();

        Stress stress5 = new Stress(50) {
                protected void runTest() throws InterruptedException {
                    testStaticThreadAsyncSyncOrder();
                }
            };
        stress5.start();

        Stress stress6 = new Stress(500) {
                protected void runTest() throws InterruptedException {
                    testDispose();
                }
            };
        stress6.start();

        stress1.join();
        stress2.join();
        stress3.join();
        stress4.join();
        stress5.join();
        stress6.join();
    }

    public void testAsyncSync() throws InterruptedException {
        TestWorkAt workAt = new TestWorkAt();
        ThreadId threadId = ThreadId.createFresh();
        MyWorkAt myWorkAt = new MyWorkAt( workAt );

        // queue asyncs
        for(int i = 0; i < TestWorkAt.MESSAGES; ++ i) {
            if( i == 2 )
            {
                putJob( myWorkAt, false , threadId, "asyncCall" );
            }
            putJob(workAt, false, threadId, "asyncCall");
        }

        synchronized(workAt) {
            putJob(workAt, false, threadId, "notifyme");

            while(!workAt._notified)
                workAt.wait();
        }

        assure("",
               workAt._async_counter == TestWorkAt.MESSAGES
               && myWorkAt._success);
    }

    private static void putJob(TestIWorkAt iWorkAt, boolean synchron,
                               ThreadId threadId, String operation) {
        __iThreadPool.putJob(
            new Job(iWorkAt, __iReceiver,
                    new TestMessage(synchron, __workAt_td, "oid", threadId,
                                    null, operation, null)));
    }

    private static final class TestThread extends Thread {
        ThreadId _threadId;
        Object _disposeId = new Object();
        String _message;
        IThreadPool _iThreadPool;

        TestThread() {
            this(__iThreadPool);
        }

        TestThread(IThreadPool iThreadPool) {
            _iThreadPool = iThreadPool;
        }

        public void run() {
            _threadId = _iThreadPool.getThreadId();


            try {
                synchronized(this) {
                    // notify that we are running
                    notify();

                    _iThreadPool.attach();

                    // wait until we should continue
                    wait();
                }

                _iThreadPool.enter();
            }
            catch(Throwable throwable) {
                _message = throwable.getMessage();
            }

            _iThreadPool.detach();

            synchronized(this) {
                // notify the listeners that we are dying
                notifyAll();
            }
        }
    }

    private static final class MyWorkAt implements TestIWorkAt {
        public MyWorkAt( TestWorkAt async_WorkAt ) {
            _async_WorkAt = async_WorkAt;
        }

        public void syncCall() throws Throwable
        {
            IMessage iMessage = new TestMessage(
                true, ThreadPool_Test.__workAt_td, "oid",
                __iThreadPool.getThreadId(), null, null, null);

            // marshal reply
            ThreadPool_Test.__iThreadPool.putJob(
                new Job(this, ThreadPool_Test. __iReceiver, iMessage));
        }

        public  void asyncCall() throws Throwable {
            for (int i = 0 ; i < 5 ; ++i) {
                ThreadPool_Test.__iThreadPool.attach();
                ThreadPool_Test.putJob(this, true, __iThreadPool.getThreadId(),
                                       "syncCall");
                // wait for reply
                ThreadPool_Test.__iThreadPool.enter();
                ThreadPool_Test.__iThreadPool.detach();
            }
            // async must have waited for this call
            _success = _async_WorkAt._async_counter == 2;
        }

        public void increment() throws Throwable {}

        public void notifyme() {}

        public boolean _success = false;

        private final TestWorkAt _async_WorkAt;
    }

    private static final IThreadPool __iThreadPool = ThreadPoolManager.create();
    private static final IReceiver __iReceiver = new TestReceiver();
    private static final TypeDescription __workAt_td
    = TypeDescription.getTypeDescription(TestIWorkAt.class);
}
