/*************************************************************************
 *
 *  $RCSfile: JobQueue_Test.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-23 17:07:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.lib.uno.environments.remote;


import java.util.Vector;

import java.io.IOException;
import java.io.OutputStream;


import com.sun.star.lib.uno.typedesc.TypeDescription;

import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;


public final class JobQueue_Test {
    /**
     * When set to true, enables various debugging output.
     */
    private static final boolean DEBUG = false;

    static JavaThreadPoolFactory __javaThreadPoolFactory = new JavaThreadPoolFactory();
    static IReceiver __iReceiver = new MyReceiver();
    static Object __disposeId = new Object();
    static TypeDescription __workAt_td = TypeDescription.getTypeDescription(IWorkAt.class);

    private static final class TestThread extends Thread {
        public final ThreadId _threadId
        = __javaThreadPoolFactory.getThreadId(this);
        public final Object _disposeId = new Object();
        public JobQueue _jobQueue = null;
        public String _message;

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
                _message = e.getMessage();
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


    private static boolean test_thread_leaves_jobQueue_on_dispose(
        Vector vector, int waitTime) throws Throwable
    {
        System.err.println("\t\ttest_thread_leaves_jobQueue_on_dispose with"
                           + " enter time: " + waitTime);
        TestThread t = new TestThread(waitTime);
        t.waitToStart();
        String msg = "xcxxxxxxxx";
        t._jobQueue.dispose(t._disposeId, new RuntimeException (msg));
        t.waitToTerminate();
        boolean passed = t._message.equals(msg);
        if (!passed) {
            vector.addElement("test_thread_leaves_jobQueue_on_dispose - not"
                              + " passed: message != " + msg + ", instead: "
                              + t._message);
        }
        System.err.println("\t\tpassed? " + passed);
        return passed;
    }

    static boolean test_thread_leaves_jobQueue_on_reply(
        Vector vector, int waitTime) throws Throwable
    {
        System.err.println("\t\ttest_thread_leaves_jobQueue_on_reply: "
                           + waitTime);
        TestThread t = new TestThread(waitTime);
        t.waitToStart();
        // put reply job
        t._jobQueue.putJob(
            new Job(null, __iReceiver,
                    new MyMessage(true, __workAt_td, "oid", null, null, null,
                                  null)),
            null);
        t.waitToTerminate();
        boolean passed = true;
        System.err.println("\t\tpassed? " + passed);
        return passed;
    }

    static void test_send_request(WorkAt workAt, String operation, JobQueue jobQueue) throws Throwable {
        IMessage iMessage = new MyMessage(true, __workAt_td, "oid", null, null, operation, null);

        jobQueue.putJob(new Job(workAt, __iReceiver, iMessage), __disposeId);
    }

    static void test_send_requests(WorkAt workAt, String operation, JobQueue jobQueue) throws Throwable {
        IMessage iMessage = new MyMessage(true, __workAt_td, "oid", null, null, operation, null);

        for(int i = 0; i < WorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            jobQueue.putJob(new Job(workAt, __iReceiver, iMessage), __disposeId);
        }
    }

    static boolean test_execute_jobs(Vector vector, JobQueue jobQueue) throws Throwable {
        boolean passed = true;

        WorkAt workAt = new WorkAt();

        test_send_requests(workAt, "increment", jobQueue);

        synchronized(workAt) {
            jobQueue.putJob(new Job(workAt, __iReceiver, new MyMessage(true, __workAt_td, "oid", null, null, "notifyme", null)), null);

            while(!workAt._notified)
                workAt.wait();
        }

        passed = workAt._counter == WorkAt.MESSAGES;
        if(!passed)
            vector.addElement("test_execute_jobs - not passed: workAt._counter == 20, instead:" + workAt._counter);

        return passed;
    }

    static boolean test_static_thread_executes_jobs(
        Vector vector, int waitTime) throws Throwable
    {
        System.err.println("\t\ttest_static_thread_executes_jobs: " + waitTime);
        TestThread t = new TestThread(waitTime);
        t.waitToStart();
        boolean passed = test_execute_jobs(vector, t._jobQueue);
        t._jobQueue.dispose(t._disposeId,
                            new RuntimeException("xxxxxxxxxxxxx"));
        t.waitToTerminate();
        System.err.println("\t\tpassed? " + passed);
        return passed;
    }

    static boolean test_dynamic_thread_executes_job(Vector vector) throws Throwable {
        boolean passed = true;

        Object disposeId = new Object();

        System.err.println("\t\ttest_dynamic_thread_executes_job:");

        JobQueue jobQueue = new JobQueue(__javaThreadPoolFactory, new ThreadId(), true);
        passed = test_execute_jobs(vector, jobQueue);

        System.err.println("\t\tpassed? " + passed);

        return passed;
    }

    static boolean test_async_jobQueue(Vector vector, WorkAt workAt, JobQueue async_jobQueue, ThreadId threadId) throws Throwable {
        boolean passed = true;

        // put slow async calls
        if(DEBUG) System.err.println("\t\t\tputting asyncs:");
        test_send_requests(workAt, "asyncCall", async_jobQueue);

        // put fast sync calls
        if(DEBUG) System.err.println("\t\t\tputting syncs:");
        test_send_requests(workAt, "syncCall", __javaThreadPoolFactory.getJobQueue(threadId));


        // wait until all is done
        synchronized(workAt) {
            async_jobQueue._sync_jobQueue.putJob(new Job(workAt, __iReceiver, new MyMessage(true, __workAt_td, "oid", null, null, "notifyme", null)), null);

            while(!workAt._notified)
                workAt.wait();
        }

        passed = passed && workAt.passedAsyncTest(vector);
        if(!passed)
            vector.addElement("workAt did not pass async test (sync overtook async)");

        return passed;
    }

    static boolean test_static_thread_executes_asyncs(Vector vector)
        throws Throwable
    {
        System.err.println("\t\ttest_static_thread_executes_asyncs:");
        TestThread t = new TestThread();

        // create an async queue
        JobQueue async_jobQueue = new JobQueue(__javaThreadPoolFactory,
                                               t._threadId);
        boolean tmp_passed = async_jobQueue._ref_count == 1;
        boolean passed = tmp_passed;

        t._jobQueue = __javaThreadPoolFactory.getJobQueue(t._threadId);
        tmp_passed = t._jobQueue._ref_count == 1;
        passed = passed && tmp_passed;

        t.waitToStart();

        WorkAt workAt = new WorkAt();

        tmp_passed = test_async_jobQueue(vector, workAt, async_jobQueue,
                                         t._threadId);
        passed = passed && passed;

        t._jobQueue.dispose(t._disposeId,
                            new RuntimeException("xxxxxxxxxxxxx"));
        t.waitToTerminate();

        tmp_passed = workAt._async_counter == WorkAt.MESSAGES;
        passed = passed && tmp_passed;

        tmp_passed = workAt._sync_counter == WorkAt.MESSAGES;
        passed = passed && tmp_passed;

        System.err.println("\t\tpassed? " + passed);
        return passed;
    }

    static boolean test_dynamic_thread_executes_asyncs(Vector vector) throws Throwable {
        boolean passed = true;

        System.err.println("\t\ttest_dynamic_thread_executes_asyncs:");


        ThreadId threadId = new ThreadId();
        JobQueue async_jobQueue = new JobQueue(__javaThreadPoolFactory, threadId);

        WorkAt workAt = new WorkAt();

        boolean tmp_passed = test_async_jobQueue(vector, workAt, async_jobQueue, threadId);
        passed = passed && tmp_passed;

        tmp_passed = workAt._async_counter == WorkAt.MESSAGES;
        if(vector != null && !tmp_passed)
            vector.addElement("test_dynamic_thread_executes_asyncs - not passed: worAt._async_counter == " + WorkAt.MESSAGES + ", instead:" + workAt._async_counter);
        passed = passed && tmp_passed;

        tmp_passed = workAt._sync_counter == WorkAt.MESSAGES;
        if(!tmp_passed)
            vector.addElement("test_dynamic_thread_executes_asyncs - not passed: worAt._sync_counter == " + WorkAt.MESSAGES + ",instead:" + workAt._sync_counter);

        passed = passed && tmp_passed;

        System.err.println("\t\tpassed? " + passed);

        return passed;
    }

    static public boolean test(Vector vector) throws Throwable {
        System.err.println("\tJobQueue test:");

        boolean passed = true;

        boolean tmp_passed = test_thread_leaves_jobQueue_on_dispose(vector, 0);
        passed = passed && tmp_passed;

          tmp_passed = test_thread_leaves_jobQueue_on_dispose(vector, 5000);
        passed = passed && tmp_passed;


          tmp_passed = test_thread_leaves_jobQueue_on_reply(vector, 0);
        passed = passed && tmp_passed;
        tmp_passed = test_thread_leaves_jobQueue_on_reply(vector, 5000);
        passed = passed && tmp_passed;

          tmp_passed = test_static_thread_executes_jobs(vector, 0);
        passed = passed && tmp_passed;
          tmp_passed = test_static_thread_executes_jobs(vector, 5000);
        passed = passed && tmp_passed;


          tmp_passed = test_dynamic_thread_executes_job(vector);
        passed = passed && tmp_passed;


        tmp_passed = test_static_thread_executes_asyncs(vector);
        passed = passed && tmp_passed;


          tmp_passed = test_dynamic_thread_executes_asyncs(vector);
        passed = passed && tmp_passed;

        System.err.println("\tpassed? " + passed);
        return passed;
    }

    static public void main(String args[]) throws Throwable {
        Vector vector = new Vector();
        test(vector);

        for(int i = 0; i < vector.size(); ++ i)
            System.err.println((String)vector.elementAt(i));
    }
}


