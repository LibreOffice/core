/*************************************************************************
 *
 *  $RCSfile: ThreadPool_Test.java,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-23 17:07:53 $
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


import java.io.IOException;
import java.io.OutputStream;

import java.util.Hashtable;
import java.util.Enumeration;
import java.util.Vector;


import com.sun.star.lib.uno.typedesc.TypeDescription;

import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;


class MyWorkAt implements IWorkAt
{
    private static boolean DEBUG = false;
    protected ThreadId _id;
    protected WorkAt _async_WorkAt;
    protected boolean _success = false;
    public MyWorkAt( WorkAt async_WorkAt )
        {
            _async_WorkAt = async_WorkAt;
        }
    public void syncCall() throws Throwable
        {
            if( DEBUG ) System.out.println( "reaching syncCall" );
            IMessage iMessage = new MyMessage(true, ThreadPool_Test.__workAt_td, "oid", ThreadPoolFactory.getThreadId(), null, null, null);

            // marshal reply
            ThreadPool_Test.__iThreadPool.putJob(new Job(this, ThreadPool_Test. __iReceiver, iMessage));
        }
    public  void asyncCall() throws Throwable
        {

            for( int i = 0 ; i < 5 ; i ++ )
            {
                if( DEBUG ) System.out.println( "starting asyncCall"  + _async_WorkAt._async_counter);
                ThreadPool_Test.__iThreadPool.attach();
                ThreadPool_Test.putJob(
                      this , true ,
                      ThreadPoolFactory.getThreadId() , "syncCall" );
                // wait for reply
                ThreadPool_Test.__iThreadPool.enter();
                ThreadPool_Test.__iThreadPool.detach();
                if( DEBUG ) System.out.println( "finishing asyncCall"  + _async_WorkAt._async_counter);
            }
            // async must have waited for this call
            _success = _async_WorkAt._async_counter == 2;
        }

    public  void increment() throws Throwable
        {

        }

    public  void notifyme()
        {

        }

}

public class ThreadPool_Test {
    /**
     * When set to true, enables various debugging output.
     */
    private static final boolean DEBUG = false;

    static IThreadPool __iThreadPool = null;
    static IReceiver __iReceiver = new MyReceiver();
    static TypeDescription __workAt_td = TypeDescription.getTypeDescription(IWorkAt.class);
    static Object __disposeId = new Object();


    static class TestThread extends Thread {
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
            _threadId = ThreadPoolFactory.getThreadId();


            try {
                synchronized(this) {
                    // notify that we are running
                    notify();

                    _iThreadPool.attach();

                    // wait until we should continue
                    wait();
                }

                if(DEBUG) System.err.println("entering queue");

                _iThreadPool.enter();
            }
            catch(Throwable throwable) {
                if(DEBUG) throwable.printStackTrace();

                _message = throwable.getMessage();
            }

            _iThreadPool.detach();

            synchronized(this) {
                if(DEBUG) System.err.println("dying");

                // notify the listeners that we are dying
                notifyAll();
            }
        }
    }

    static void putJob(IWorkAt iWorkAt, boolean synchron, ThreadId threadId, String operation) {
        IMessage iMessage = new MyMessage(synchron, __workAt_td, "oid", threadId, null, operation, null);

        __iThreadPool.putJob(new Job(iWorkAt, __iReceiver, iMessage));
    }

    private static void test_brokenImplementation() {
        // The implementation of JavaThreadPoolFactory (rev. 1.2) and JobQueue
        // (rev. 1.15) contains the following code as a race condition:
        // 1  A call to JobQueue.acquire leads to
        //    _javaThreadPoolFactory.addJobQueue(this), leads to a
        //    _jobQueues.put call.
        // 2  A call to JavaThreadPoolFactory.dispose creates an Enumeration;
        //    assume that, between calls to Enumeration.hasMoreElements and
        //    Enumeration.nextElement, JobQueue.release is called, leading to
        //    _javaThreadPoolFactory.removeJobQueue(this), leading to a
        //    _jobQueues.remove call; the use of the return value of the
        //    following Enumeration.nextElement results in a
        //    NullPointerException.
        // (Detected once while running test_stress.)
        Hashtable ht = new Hashtable();
        String key = "key";
        Object entry = new Object();
        ht.put(key, entry);
        Enumeration e = ht.elements();
        if (!e.hasMoreElements()) {
            throw new IllegalStateException();
        }
        ht.remove(key);
        if (e.nextElement() != null) {
            throw new IllegalStateException();
        }
    }

    static boolean test_dispose(Vector vector, boolean silent) throws Throwable {
        boolean passed = true;

        if(!silent)
            System.err.println("\t\ttest_dispose:");

        IThreadPool iThreadPool = ThreadPoolFactory.createThreadPool();
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
            if(DEBUG) System.err.println("waiting for TestThread to die");

            // put reply job
            iThreadPool.dispose(new RuntimeException(message));

            testThread.wait();
        }

        testThread.join();


        passed = testThread._message.equals(message);

        if(!silent)
            System.err.println("\t\tpassed? " + passed);

        return passed;
    }



    static boolean test_thread_async(Vector vector, boolean silent) throws Throwable {
        boolean passed = true;

        if(!silent)
            System.err.println("\t\ttest_thread_async:");

        WorkAt workAt = new WorkAt();


        ThreadId threadId = new ThreadId();

        // queue asyncs
        for(int i = 0; i < WorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, false, threadId, "increment");
        }

        synchronized(workAt) {
            putJob(workAt, false, threadId, "notifyme");

            while(!workAt._notified)
                workAt.wait();
        }

        passed = workAt._counter == WorkAt.MESSAGES;

        if(!silent)
            System.err.println("\t\tpassed? " + passed);

        return passed;
    }

    static boolean test_dynamic_thread_sync(Vector vector, boolean silent) throws Throwable {
        boolean passed = true;

        if(!silent)
            System.err.println("\t\t test_dynamic_thread_sync:");

        WorkAt workAt = new WorkAt();


        ThreadId threadId = new ThreadId();

        // queue asyncs
        for(int i = 0; i < WorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, true, threadId, "increment");
        }

        synchronized(workAt) {
            putJob(workAt, true, threadId, "notifyme");

            while(!workAt._notified)
                workAt.wait();
        }

        passed = workAt._counter == WorkAt.MESSAGES;

        if(!silent)
            System.err.println("\t\tpassed? " + passed);

        return passed;
    }

    static boolean test_static_thread_sync(Vector vector, boolean silent) throws Throwable {
        boolean passed = true;

        if(!silent)
            System.err.println("\t\t test_static_thread_sync:");

        WorkAt workAt = new WorkAt();

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
        for(int i = 0; i < WorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, true, threadId, "increment");
        }


        // terminate the test thread
        synchronized(testThread) {
            if(DEBUG) System.err.println("waiting for TestThread to die");

            // put reply job
            putJob(workAt, true, threadId, null);

            testThread.wait();
        }

        testThread.join();


        passed = workAt._counter == WorkAt.MESSAGES;

        if(!silent)
            System.err.println("\t\tpassed? " + passed);

        return passed;
    }

    static boolean test_dynamic_thread_async_sync_order(Vector vector, boolean silent) throws Throwable {
        boolean passed = true;

        if(!silent)
            System.err.println("\t\ttest_dynamic_thread_async_sync_order:");

        WorkAt workAt = new WorkAt();


        ThreadId threadId = new ThreadId();

        // queue asyncs
        for(int i = 0; i < WorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, false, threadId, "asyncCall");
        }

        // queue syncs
        for(int i = 0; i < WorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, true, threadId, "syncCall");
        }


        synchronized(workAt) {
            putJob(workAt, true, threadId, "notifyme");

            while(!workAt._notified)
                workAt.wait();
        }

        passed = workAt.passedAsyncTest(vector);

        if(!silent)
            System.err.println("\t\tpassed? " + passed);

        return passed;
    }



    static boolean test_static_thread_async_sync_order(Vector vector, boolean silent) throws Throwable {
        boolean passed = true;

        if(!silent)
            System.err.println("\t\ttest_static_thread_async_sync_order:");

        WorkAt workAt = new WorkAt();

        TestThread testThread = new TestThread();

        // start the test thread
        synchronized(testThread) {
            testThread.start();

            testThread.wait();
        }

        ThreadId threadId = testThread._threadId;

        // queue asyncs
        for(int i = 0; i < WorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, false, threadId, "asyncCall");
        }


        // let the thread attach and enter the threadpool
        synchronized(testThread) {
            testThread.notifyAll();
        }


        // queue syncs
        for(int i = 0; i < WorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            putJob(workAt, true, threadId, "syncCall");
        }


        // terminate the test thread
        synchronized(testThread) {
            if(DEBUG) System.err.println("waiting for TestThread to die");

            // put reply job
            putJob(workAt, true, threadId, null);

            testThread.wait();
        }

        testThread.join();

        passed = workAt.passedAsyncTest(vector);

        if(!silent)
            System.err.println("\t\tpassed? " + passed);

        return passed;
    }

    static boolean test_async_sync( Vector vector , boolean silent ) throws InterruptedException
    {
        boolean passed = true;

        if(!silent)
            System.err.println("\t\ttest_async_sync:");

        WorkAt workAt = new WorkAt();
        ThreadId threadId = new ThreadId();
        MyWorkAt myWorkAt = new MyWorkAt( workAt );

        // queue asyncs
        for(int i = 0; i < WorkAt.MESSAGES; ++ i) {
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

        passed = workAt._async_counter == WorkAt.MESSAGES && myWorkAt._success;

        if(!silent)
            System.err.println("\t\tpassed? " + passed);

        return passed;
    }

    static boolean test_stress(Vector vector) throws Throwable {
        boolean passed = true;

        System.err.println("\t\ttest_stress:");

        WorkAt workAt = new WorkAt();

        for(int i = 0; i < WorkAt.MESSAGES; ++ i) {
            Thread.yield(); // force scheduling
            ThreadId threadID = new ThreadId();

            putJob(workAt, true, threadID, "increment");
            putJob(workAt, false, threadID, "increment");
        }


        synchronized(workAt) {
            while(workAt._counter < (2 * WorkAt.MESSAGES))
                workAt.wait();
        }


        class Stress1 extends Thread {
            Vector _vector;
            boolean _passed = true;

            Stress1(Vector vector) {
                _vector = vector;
            }

            public void run() {
                try {
                    for(int i = 0; i < 50; ++ i) {
                        boolean tmp_passed = test_thread_async(_vector, true);

                        _passed = _passed && tmp_passed;
                    }
                }
                catch(Throwable throwable) {
                    System.err.println(throwable);
                    throwable.printStackTrace();
                }
            }
        };


        Stress1 stress1 = new Stress1(vector);
        stress1.start();

        class Stress2 extends Thread {
            Vector _vector;
            boolean _passed = true;

            Stress2(Vector vector) {
                _vector = vector;
            }

            public void run() {
                try {
                    for(int i = 0; i < 50; ++ i) {
                        boolean tmp_passed = test_dynamic_thread_sync(_vector, true);

                        _passed = _passed && tmp_passed;
                    }
                }
                catch(Throwable throwable) {
                    System.err.println(throwable);
                    throwable.printStackTrace();
                }
            }
        };


        Stress2 stress2 = new Stress2(vector);
        stress2.start();



        class Stress3 extends Thread {
            Vector _vector;
            boolean _passed = true;

            Stress3(Vector vector) {
                _vector = vector;
            }

            public void run() {
                try {
                    for(int i = 0; i < 50; ++ i) {
                        boolean tmp_passed = test_static_thread_sync(_vector, true);

                        _passed = _passed && tmp_passed;
                    }
                }
                catch(Throwable throwable) {
                    System.err.println(throwable);
                    throwable.printStackTrace();
                }
            }
        };


        Stress3 stress3 = new Stress3(vector);
          stress3.start();



        class Stress4 extends Thread {
            Vector _vector;
            boolean _passed = true;

            Stress4(Vector vector) {
                _vector = vector;
            }

            public void run() {
                try {
                    for(int i = 0; i < 50; ++ i) {
                        boolean tmp_passed = test_dynamic_thread_async_sync_order(_vector, true);

                        _passed = _passed && tmp_passed;
                    }
                }
                catch(Throwable throwable) {
                    System.err.println(throwable);
                    throwable.printStackTrace();
                }
            }
        };


        Stress4 stress4 = new Stress4(vector);
          stress4.start();


        class Stress5 extends Thread {
            Vector _vector;
            boolean _passed = true;

            Stress5(Vector vector) {
                _vector = vector;
            }

            public void run() {
                try {
                    for(int i = 0; i < 50; ++ i) {
                        boolean tmp_passed = test_static_thread_async_sync_order(_vector, true);

                        _passed = _passed && tmp_passed;
                    }
                }
                catch(Throwable throwable) {
                    System.err.println(throwable);
                    throwable.printStackTrace();
                }
            }
        };


        Stress5 stress5 = new Stress5(vector);
        stress5.start();


        class Stress6 extends Thread {
            Vector _vector;
            boolean _passed = true;

            Stress6(Vector vector) {
                _vector = vector;
            }

            public void run() {
                for(int i = 0; i < 500; ++ i) {
//                          Thread.sleep(500);
                    try {
                        boolean tmp_passed = test_dispose(_vector, true);

                        _passed = _passed && tmp_passed;
                    }
                    catch(Throwable throwable) {
                        System.err.println(throwable);
                        throwable.printStackTrace();

                        _passed = false;
                        _vector.addElement("Stress6 - exception:" + throwable);
                    }
                }
            }
        };


        Stress6 stress6 = new Stress6(vector);
          stress6.start();




        stress1.join();
        stress2.join();
        stress3.join();
        stress4.join();
        stress5.join();

        if(!stress1._passed)
            vector.addElement("Stress1 not passed");

        if(!stress2._passed)
            vector.addElement("Stress2 not passed");

        if(!stress3._passed)
            vector.addElement("Stress3 not passed");

        if(!stress4._passed)
            vector.addElement("Stress4 not passed");

        if(!stress5._passed)
            vector.addElement("Stress5 not passed");

        if(!stress6._passed)
            vector.addElement("Stress6 not passed");

        passed = passed && stress1._passed;
        passed = passed && stress2._passed;
        passed = passed && stress3._passed;
        passed = passed && stress4._passed;
        passed = passed && stress5._passed;
        passed = passed && stress6._passed;

        System.err.println("\t\tpassed? " + passed);

        return passed;
    }

    static public boolean test(Vector vector) throws Throwable {
        __iThreadPool = ThreadPoolFactory.createThreadPool();

        System.err.println("\tThreadPool test:");

        boolean passed = true;
        boolean tmp_passed = false;

        test_brokenImplementation();

          tmp_passed = test_dispose(vector, false);
        passed = passed && tmp_passed;

          tmp_passed = test_thread_async(vector, false);
        passed = passed && tmp_passed;

          tmp_passed = test_dynamic_thread_sync(vector, false);
        passed = passed && tmp_passed;

          tmp_passed = test_static_thread_sync(vector, false);
        passed = passed && tmp_passed;

          tmp_passed = test_dynamic_thread_async_sync_order(vector, false);
        passed = passed && tmp_passed;

          tmp_passed = test_static_thread_async_sync_order(vector, false);
        passed = passed && tmp_passed;

           tmp_passed = test_stress(vector);
          passed = passed && tmp_passed;

        tmp_passed = test_async_sync(vector,false);
        passed = passed && tmp_passed;


        System.err.println("\tpassed? " + passed);
        return passed;
    }

    static public void main(String args[]) throws Throwable {
        Vector vector = new Vector();

        test(vector);

        for(int i = 0; i < vector.size(); ++ i)
            System.err.println("---:" + vector.elementAt(i));
    }
}
