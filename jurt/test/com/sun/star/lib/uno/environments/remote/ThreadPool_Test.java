/*************************************************************************
 *
 *  $RCSfile: ThreadPool_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2000-09-28 11:36:14 $
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

import java.util.Enumeration;
import java.util.Vector;


import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.UnoRuntime;


public class ThreadPool_Test {
    static int __requestId = 0;
    static int __running_thread_count;

    static interface MyInterface {
        Object syncCall(Object param) throws Exception;
        Object asyncCall(Object param) throws Exception;
    }

    static interface IReadyListener {
        void readyEvent();
    }


    static class MyImpl implements MyInterface {
        int _received_requestId;
        int _send_requestId;
        boolean _passed = true;
        IReadyListener _iReadyListener;
        boolean _block;

        MyImpl() {
        }

        int getNext() {
            return _send_requestId ++;
        }

        void addReadyListener(IReadyListener iReadyListener) {
            _iReadyListener = iReadyListener;
        }

        public Object syncCall(Object param) throws Exception{
            Object object =  doSomething(param);

            // send a request to ourself

//              ThreadPool.addThread(null);
//              Job job = new Job(new MyReceiver(null),      // receiver
//                                JavaThreadPool.getThreadId(Thread.currentThread()),        // threadID
//                                __requestId ++,     // requestId
//                                this,          // object
//                                null,   // operation,
//                                new MyMessage(0),       // parameter
//                                true,            // synchron ?
//                                null,                // exception ?
//                                MyInterface.class);  // interface


//              ThreadPool.putJob(job, null);
//              System.err.println("entering ...");
//              ThreadPool.enter();
//              System.err.println("left");
            if(_block) {
                try {
                    synchronized(this) {
                        System.err.println(this + " waiting for interrupt...");
                        wait(); // wait for exception
                    }
                }
                catch(InterruptedException interruptedException) {
                    System.err.println(this + " succecessfully interrupted - rethrowing...");
                    throw interruptedException;
                }
            }

            return object;

        }

        public Object asyncCall(Object param) throws Exception {
            return doSomething(param);
        }

        Object doSomething(Object param) throws Exception {
//              synchronized(this) {
//                  long waitTime = (long)(Math.random() * 100);
//                  if(waitTime > 0)
//                      wait(waitTime); // simulate work
//              }

            _passed = _passed && (((Integer)param).intValue() == _received_requestId);

            if(!_passed)
                throw new NullPointerException("blblbl");




            ++ _received_requestId;

            if(_iReadyListener != null)
                _iReadyListener.readyEvent();

//              synchronized(this) {
//                  long waitTime = (long)(Math.random() * 100);
//                  if(waitTime > 0)
//                      wait(waitTime); // simulate work
//              }

            return "blabla";
        }

        void finish() {
            _passed = _passed && (_send_requestId == _received_requestId);
        }

        boolean pendingRequests() {
            return _send_requestId != _received_requestId;
        }
    }



    static void sendJobs(int jobs, MyReceiver myReceiver, ThreadID threadID, MyImpl myImpl, boolean synchron, boolean finish, Object disposeId) throws Exception {
        // sending synchronous calls
        System.err.println("sending " + jobs + " " + synchron + " calls...");

        for(int i = 0; i < jobs; ++ i) {
            MyMessage myMessage = new MyMessage(synchron,
                                                MyInterface.class,
                                                UnoRuntime.generateOid(myImpl),
                                                threadID,
                                                myImpl,
                                                finish ? null : (synchron ? "syncCall": "asyncCall"),
                                                new Object[]{new Integer(myImpl.getNext())});

            Job job = new Job(myImpl, myReceiver, myMessage);
//              Job job = new Job(UnoRuntime.generateOid(myImpl),
//                                myReceiver,      // receiver
//                                threadID,        // threadID
//                                __requestId ++,     // requestId
//                                myImpl,          // object
//                                finish ? null : (synchron ? "syncCall": "asyncCall"),   // operation,
//                                myMessage,       // parameter
//                                synchron,            // synchron ?
//                                null,                // exception ?
//                                MyInterface.class);  // interface

            ThreadPool.putJob(job, disposeId);
        }
    }



    static class Worker_with_Thread extends Thread {
        MyImpl _myImpl         = new MyImpl();
        MyReceiver _myReceiver = new MyReceiver(null);
        boolean _started = false;

        public void run() {
            System.err.println("WorkerThread - started");
            try {
                ThreadPool.addThread(null);
                _started = true;

                ThreadPool.enter();

//                  _myImpl.finish();
            }
            catch(Exception exception) {
                System.err.println("WorkerThread - exception:" + exception);
                exception.printStackTrace();
            }
            System.err.println("WorkerThread - finished - passed:" + _myImpl._passed);
        }
    }

    static class RemoteObject implements IReadyListener {
        MyImpl _myImpl         = new MyImpl();
        MyReceiver _myReceiver = new MyReceiver(null);
        ThreadID _threadID = new ThreadID(UnoRuntime.generateOid(this).getBytes());

        boolean _finished = false;

        {
            _myImpl.addReadyListener(this);
        }

        public synchronized void readyEvent() {
            _finished = true;
            notifyAll();
        }
    }

    static Vector __threads = new Vector();


    static class SenderThread extends Thread {
        boolean _quit = false;
        Object  _disposeId;

        SenderThread(Object disposeId) {
            _disposeId = disposeId;
        }

        public void run() {
            try {
                while(!_quit && __threads.size() > 0) {
                    Enumeration elements = __threads.elements();
                    while(elements.hasMoreElements() && !_quit) {
                        Object object = elements.nextElement();

                        if(object instanceof Worker_with_Thread) {
                            Worker_with_Thread thread = (Worker_with_Thread)object;

                            if(thread._started && !thread._myImpl.pendingRequests()) {
                                sendJobs((int)(Math.random() * 50 + 1), thread._myReceiver, JavaThreadPool.getThreadId(thread), thread._myImpl, false, false, _disposeId);
                                sendJobs((int)(1), thread._myReceiver, JavaThreadPool.getThreadId(thread), thread._myImpl, true, false, _disposeId);

                                if(Math.random() > 0.95) {
                                    sendJobs(1, thread._myReceiver, JavaThreadPool.getThreadId(thread), thread._myImpl, true, true, _disposeId); // finish

                                    __threads.removeElement(thread);
                                }
                            }
                        }
                        else {
                            RemoteObject remoteObject = (RemoteObject)object;

                            if(!remoteObject._myImpl.pendingRequests()) {
                                sendJobs((int)(Math.random() * 50 + 1), remoteObject._myReceiver, remoteObject._threadID, remoteObject._myImpl, false, false, _disposeId);
                                sendJobs((int)(Math.random() * 50 + 1), remoteObject._myReceiver, remoteObject._threadID, remoteObject._myImpl, true, false, _disposeId);

                                if(Math.random() > 0.95) {
                                    __threads.removeElement(remoteObject);
                                }
                            }
                        }
                    }
                    synchronized(this) {
                        notify();
                    }
//                      Thread.sleep((int)(Math.random() * 100));
                }
            }
            catch(Exception exception) {
                System.err.println("SenderThread - exception:" + exception);
                exception.printStackTrace();
            }
        }
    }

    static public boolean test_with_thread() throws InterruptedException {
        Object disposeId = new Integer(0);

        int blockers = 0;

        SenderThread senderThread = new SenderThread(disposeId);
//          senderThread.start();
        boolean started = false;
        Vector threads = new Vector();

        do {
            ++ __running_thread_count;
            Object object = null;

            if(Math.random() > 1.25) {
                Thread thread = new Worker_with_Thread();
                thread.start();

                object = thread;
            }
            else {
                object = new RemoteObject();

                if(Math.random() > 0.70) {
                    ((RemoteObject)object)._myImpl._block = true;
                    ++ blockers;
                }
            }

            __threads.addElement(object);
            threads.addElement(object);

            if(!started) {
                started = true;
                senderThread.start();
            }

            Thread.sleep((int)(Math.random() * 1000));
        }
        while(Math.random() > 0.05);


        System.err.println("waiting for SenderThread to die...");
        senderThread._quit = true;
        senderThread.join();

        System.err.println("disposing ThreadPool for id " + disposeId + " with " + blockers + " blocked ...");
        ThreadPool.dispose(disposeId);

        boolean passed = true;

        // wait for all threads
        System.err.println("joining all threads ...");
        Enumeration elements = threads.elements();
        while(elements.hasMoreElements()) {
            Object object = elements.nextElement();
            if(object instanceof Worker_with_Thread) {
                Worker_with_Thread thread = (Worker_with_Thread)object;
                thread.join();

                passed = passed && thread._myImpl._passed;
            }
            else {
                RemoteObject remoteObject = (RemoteObject)object;
                synchronized(remoteObject) {
                    while(!remoteObject._finished && !remoteObject._myImpl._block && remoteObject._myImpl._send_requestId > 0)
                        remoteObject.wait(100);
                }
            }
        }

        System.err.println("test_with_thread - passed? " + passed);

        return passed;
    }

    static public boolean test(Vector notpassed) throws Exception {
        boolean passed = true;

        passed = passed && test_with_thread();
        if(!passed && notpassed != null)
            notpassed.addElement("ThreadPool_Test - test_with_thread passed?" + passed);

        return passed;
    }

    static public void main(String args[]) throws Exception {
        test(null);
    }
}


