/*************************************************************************
 *
 *  $RCSfile: JobQueue_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2000-09-28 11:33:47 $
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

import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.UnoRuntime;


public class JobQueue_Test {
    static interface MyInterface {
        int getNext();

        Object syncCall(Object param) throws Exception;
        Object asyncCall(Object param) throws Exception;
    }

    static class MyContext {
    }

    static class MyImpl implements MyInterface {
        int _received_requestId;
        int _send_requestId;
        boolean _passed = true;
        Object _context = new MyContext();

        public int getNext() {
            return _send_requestId ++;
        }

        public Object doSomething(Object param) throws InterruptedException {
 //             synchronized(this) {
//                  long waitTime = (long)(Math.random() * 100);
//                  if(waitTime > 0)
//                      wait(waitTime); // simulate work
//              }

            _passed = _passed && (((Integer)param).intValue() == _received_requestId);

            if(!_passed)
                throw new NullPointerException("blblbl");

            ++ _received_requestId;


//              synchronized(this) {
//                  long waitTime = (long)(Math.random() * 100);
//                  if(waitTime > 0)
//                      wait(waitTime); // simulate work
//              }

            return "blabla";
        }

        public Object syncCall(Object param) throws Exception{
            Object object =  doSomething(param);

            // send a request to ourself

            ThreadPool.addThread(_context);
            Job job = new Job(this, new MyReceiver(null),      // receiver
                              new MyMessage(true,
                                            MyInterface.class,
                                            UnoRuntime.generateOid(this),
                                            JavaThreadPool.getThreadId(Thread.currentThread()),
                                            this,
                                            null,
                                            null));

            ThreadPool.putJob(job, null);
//              System.err.println("entering ...");
            ThreadPool.enter();
//              System.err.println("left");

            return object;

        }

        public Object asyncCall(Object param) throws Exception {
            return doSomething(param);
        }

        void finish() {
            _passed = _passed && (_send_requestId == _received_requestId);
        }
    }


    // this is for testing dispose
    static class MyImpl2 implements MyInterface {
        int _received_requestId;
        int _send_requestId;
        boolean _passed = true;
        Object _context = new MyContext();


        public int getNext() {
            return _send_requestId ++;
        }

        boolean waitForException() {
            boolean result = false;

            try {
                synchronized(this) {
                    notify(); // notify the tester that we entered the call
                    wait();   // wait for tester to tell us to leave
                }
            }
            catch(InterruptedException interruptedException) {
                result = true;
            }

            return result;
        }

        public Object syncCall(Object param) throws Exception{
            System.err.println("\tsyncCall - waiting for exception...");
            boolean occurred = waitForException();
            System.err.println("\toccurred (should):" + occurred);

            _passed = _passed && occurred;

            synchronized(this) {
                notify();
            }

            return "hallo";
        }

        public Object asyncCall(Object param) throws Exception {
            System.err.println("\tasyncCall - waiting for exception...");
            boolean occurred = waitForException();
            System.err.println("\toccurred (should not):" + occurred);

            _passed = _passed && !occurred;

            synchronized(this) {
                notify();
            }

            return "hallo";
        }
    }




    static void sendAsyncJobs(int jobs, JobQueue jobQueue, MyReceiver myReceiver, ThreadID threadID, MyInterface myImpl, Object context) {
        // sending asynchrones calls
        System.err.println("\tsending " + jobs + " asynchrones calls...");

        for(int i = 0; i < jobs; ++ i) {
            MyMessage myMessage   = new MyMessage(false,
                                                  MyInterface.class,
                                                  UnoRuntime.generateOid(myImpl),
                                                  threadID,
                                                  myImpl,
                                                  "asyncCall",
                                                  new Object[]{new Integer(myImpl.getNext())});

            Job job = new Job(myImpl, myReceiver, myMessage);

            jobQueue.putJob(job, context);
        }
    }

    static void sendSyncJobs(int jobs, JobQueue jobQueue, MyReceiver myReceiver, ThreadID threadID, MyInterface myImpl, Thread thread, Object context, boolean wait) throws Exception {
        // sending synchronous calls
        System.err.println("\tsending " + jobs + " synchrones calls...");

        for(int i = 0; i < jobs; ++ i) {
            MyMessage myMessage   = new MyMessage(true,
                                                  MyInterface.class,
                                                  UnoRuntime.generateOid(myImpl),
                                                  threadID,
                                                  myImpl,
                                                  "syncCall",
                                                  new Object[]{new Integer(myImpl.getNext())});


            Job job_do = new Job(myImpl, myReceiver, myMessage);

            job_do._disposeId = context;
            if(thread == null) {
                synchronized(myReceiver) {
                    jobQueue.putJob(job_do, context);

                    if(wait) {  // wait for the answer?
                        myReceiver.wait();
                    }
                }
            }
            else {
                jobQueue.putJob(job_do, context);

                myMessage   = new MyMessage(true,
                                            MyInterface.class,
                                            UnoRuntime.generateOid(myImpl),
                                            threadID,
                                            myImpl,
                                            null,
                                            null);
                Job job_return = new Job(myImpl, myReceiver, myMessage);

                jobQueue.putJob(job_return, context);
                jobQueue.enter(context);
            }
        }
    }

    static public boolean test_without_thread() throws Exception { // this is like sending jobs from remote
        boolean passed[] = new boolean[]{true};

        System.err.println("doing test_without_thread ...");

        ThreadID threadID = new ThreadID("test_thread_id".getBytes());

        Object context = new MyContext();

        JobQueue jobQueue     = new JobQueue(threadID, true);
        MyImpl myImpl         = new MyImpl();
        MyReceiver myReceiver = new MyReceiver(passed);

        do {
            sendAsyncJobs((int)(Math.random() * 10 + 1), jobQueue, myReceiver, threadID, myImpl, context);
            sendSyncJobs((int)(Math.random() * 10 + 1), jobQueue, myReceiver, threadID, myImpl, null, context, true);
            sendAsyncJobs((int)(Math.random() * 10 + 1), jobQueue, myReceiver, threadID, myImpl, context);
            sendSyncJobs((int)(Math.random() * 10 + 1), jobQueue, myReceiver, threadID, myImpl, null, context, true);
        }
        while(Math.random() > 0.25);


        myImpl.finish();

        passed[0] = passed[0] && myImpl._passed;

        System.err.println("test_without_thread - passed? " + passed[0]);

        jobQueue.printStats();

        return passed[0];
    }

    static public boolean test_with_thread() throws Exception {
        boolean passed[] = new boolean[]{true};

        System.err.println("doing test_with_thread ...");

        ThreadID threadID = new ThreadID("test_thread_id".getBytes());

        Object context = new MyContext();

        Thread thread = Thread.currentThread();

        JobQueue jobQueue     = new JobQueue(threadID, false);
        MyImpl myImpl         = new MyImpl();
        MyReceiver myReceiver = new MyReceiver(passed);

        do {
            sendAsyncJobs((int)(Math.random() * 10 + 1), jobQueue, myReceiver, threadID, myImpl, context);
            sendSyncJobs((int)(Math.random() * 10 + 1), jobQueue, myReceiver, threadID, myImpl, thread, context, true);
            sendAsyncJobs((int)(Math.random() * 10 + 1), jobQueue, myReceiver, threadID, myImpl, context);
            sendSyncJobs((int)(Math.random() * 10 + 1), jobQueue, myReceiver, threadID, myImpl, thread, context, true);
        }
        while(Math.random() > 0.25);


        myImpl.finish();

        passed[0] = passed[0] && myImpl._passed;

        System.err.println("test_with_thread - passed? " + passed[0]);

        jobQueue.printStats();

        return passed[0];
    }



    static public boolean test_disposing() throws Exception {
        boolean passed[] = new boolean[]{true};

        System.err.println("doing test_disposing ...");

        ThreadID threadID = new ThreadID("test_thread_id".getBytes());

        Object context = new MyContext();

        JobQueue jobQueue     = new JobQueue(threadID, true);
        MyImpl2 myImpl         = new MyImpl2();
        MyReceiver myReceiver = new MyReceiver(passed);


        // see if asyncs are interruptable, they should not be
        synchronized(myImpl) {
            sendAsyncJobs(1, jobQueue, myReceiver, threadID, myImpl, context);
            myImpl.wait();
            jobQueue.interrupt(context);
            myImpl.notify();
            myImpl.wait();
        }

        // see if syncs are interruptable, they should be
        synchronized(myImpl) {
            sendSyncJobs(1, jobQueue, myReceiver, threadID, myImpl, null, context, false);
            myImpl.wait();
            jobQueue.interrupt(context);
            myImpl.notify();
            myImpl.wait();
        }

        passed[0] = passed[0] && myImpl._passed;

        System.err.println("test_disposing - passed? " + passed[0]);

        jobQueue.printStats();

        return passed[0];
    }


    static public boolean test(Vector notpassed) throws Exception {
        boolean passed = true;

        passed = passed && test_without_thread();
        if(!passed && notpassed != null)
            notpassed.addElement("JobQueue_Test - test_without_thread passed?" + passed);

        passed = passed && test_with_thread();
        if(!passed && notpassed != null)
            notpassed.addElement("JobQueue_Test - test_with_thread passed?" + passed);

        passed = passed && test_disposing();
        if(!passed && notpassed != null)
            notpassed.addElement("JobQueue_Test - test_disposing passed?" + passed);

        return passed;
    }

    static public void main(String args[]) throws Exception {
        test(null);
    }
}


