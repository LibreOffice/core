/*************************************************************************
 *
 *  $RCSfile: JobQueue_Test.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kr $ $Date: 2001-05-17 12:55:06 $
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


public class JobQueue_Test {
    /**
     * When set to true, enables various debugging output.
     */
    private static final boolean DEBUG = false;

    static JavaThreadPoolFactory __javaThreadPoolFactory = new JavaThreadPoolFactory();
    static IReceiver __iReceiver = new MyReceiver();
    static Object __disposeId = new Object();
    static TypeDescription __workAt_td = TypeDescription.getTypeDescription(IWorkAt.class);

//      static class Receiver implements IReceiver {
//          public void sendReply(boolean exception, ThreadID threadId, Object result) {
//  //              System.err.println(getClass().getName() + ".sendReply " + threadId + " " + result);
//          }
//      }



    static class TestThread extends Thread {
        ThreadId _threadId = __javaThreadPoolFactory.getThreadId(this);
        JobQueue _jobQueue;
        Object _disposeId = new Object();
        int _waitTime_before_enter;
        String _message;

        TestThread(int waitTime_before_enter) {
            _waitTime_before_enter = waitTime_before_enter;

            _jobQueue = new JobQueue(__javaThreadPoolFactory, _threadId, false);
        }

        TestThread(JobQueue jobQueue) {
            _jobQueue = jobQueue;

        }

        public void run() {

            synchronized(this) {
                notify();
            }

            try {
                if(_waitTime_before_enter != 0)
                    Thread.sleep(_waitTime_before_enter);

                if(DEBUG)System.err.println("entering queue");

                _jobQueue.enter(_disposeId);
            }
            catch(Throwable throwable) {
                if(DEBUG) {
                    System.err.println("throwable:" + throwable);
                    throwable.printStackTrace();
                }

                _message = throwable.getMessage();
            }

            synchronized(this) {
                if(DEBUG) System.err.println("dying");

                // notify the listeners that we are dying
                notifyAll();
            }
        }
    }


    static boolean test_thread_leaves_jobQueue_on_dispose(Vector vector, int waitTime_before_enter) throws Throwable {
        boolean passed = true;

        System.err.println("\t\ttest_thread_leaves_jobQueue_on_dispose with enter time:" + waitTime_before_enter);

        TestThread testThread = new TestThread(waitTime_before_enter);
        synchronized(testThread) {
            testThread.start();

            testThread.wait();
        }

        String message = "xcxxxxxxxx";

        synchronized(testThread) {
            testThread._jobQueue.dispose(testThread._disposeId, new RuntimeException(message));

            if(DEBUG) System.err.println("waiting for TestThread to die");

            testThread.wait();
        }

        testThread.join();

        passed = testThread._message.equals(message);
        if(!passed)
            vector.addElement("test_thread_leaves_jobQueue_on_dispose - not passed: message != " + message + ", instead:" + testThread._message);

        System.err.println("\t\tpassed? " + passed);

        return passed;
    }

    static boolean test_thread_leaves_jobQueue_on_reply(Vector vector, int waitTime_before_enter) throws Throwable {
        boolean passed = true;

        System.err.println("\t\ttest_thread_leaves_jobQueue_on_reply:" + waitTime_before_enter);
        TestThread testThread = new TestThread(waitTime_before_enter);
        synchronized(testThread) {
            testThread.start();

            testThread.wait();
        }

        synchronized(testThread) {
            if(DEBUG) System.err.println("waiting for TestThread to die");

            // put reply job
            testThread._jobQueue.putJob(new Job(null, __iReceiver, new MyMessage(true, __workAt_td, "oid", null, null, null, null)), null);

            testThread.wait();
        }

        testThread.join();

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


    static boolean test_static_thread_executes_jobs(Vector vector, int waitTime_before_enter) throws Throwable {
        boolean passed = true;

        System.err.println("\t\ttest_static_thread_executes_jobs:" + waitTime_before_enter);
        TestThread testThread = new TestThread(waitTime_before_enter);
        synchronized(testThread) {
            testThread.start();

            testThread.wait();
        }

        passed = test_execute_jobs(vector, testThread._jobQueue);

        testThread._jobQueue.dispose(testThread._disposeId, new RuntimeException("xxxxxxxxxxxxx"));

        synchronized(testThread) {
            if(DEBUG) System.err.println("waiting for TestThread to die");

            testThread.wait();
        }

        testThread.join();

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

    static boolean test_static_thread_executes_asyncs(Vector vector) throws Throwable {
        boolean passed = true;

        System.err.println("\t\ttest_static_thread_executes_asyncs:");

        TestThread testThread = new TestThread(null);

        // create an async queue
        JobQueue async_jobQueue = new JobQueue(__javaThreadPoolFactory, testThread._threadId);
        boolean tmp_passed = async_jobQueue._ref_count == 1;
        passed = passed && tmp_passed;


        testThread._jobQueue = __javaThreadPoolFactory.getJobQueue(testThread._threadId);
        tmp_passed = testThread._jobQueue._ref_count == 1;
        passed = passed && tmp_passed;


        synchronized(testThread) {
            testThread.start();

            testThread.wait();
        }

        WorkAt workAt = new WorkAt();

        tmp_passed = test_async_jobQueue(vector, workAt, async_jobQueue, testThread._threadId);
        passed = passed && passed;

        testThread._jobQueue.dispose(testThread._disposeId, new RuntimeException("xxxxxxxxxxxxx"));
        testThread.join();

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


