/*************************************************************************
 *
 *  $RCSfile: JobQueue.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kr $ $Date: 2000-10-19 15:44:57 $
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


import java.util.Hashtable;

import com.sun.star.uno.UnoRuntime;


/**
 * The <code>JobQueue</code> implements a queue for jobs.
 * For every jobs thread id exists a job queue which is registered
 * at the <code>ThreadPool</code>.
 * A JobQueue is splitted in a sync job queue and an async job queue.
 * The sync job queue is the registerd queue, it delegates async jobs
 * (put by <code>putjob</code>) into the async queue, which is only
 * known by the sync queue.
 * <p>
 * @version     $Revision: 1.3 $ $ $Date: 2000-10-19 15:44:57 $
 * @author      Kay Ramme
 * @see         com.sun.star.lib.uno.environments.remote.ThreadPool
 * @see         com.sun.star.lib.uno.environments.remote.Job
 * @see         com.sun.star.lib.uno.environments.remote.ThreadID
 * @since       UDK1.0
 */
public class JobQueue {
    /**
     * When set to true, enables various debugging output.
     */
    public static final boolean DEBUG = false;

    static protected int __instances;

    protected Job _head;                 // the head of the job list
    protected Job _tail;                 // the tail of the job list
    protected Job _current;              // the current executing job

//      protected int _todo = 0;             // jobs to do

    protected ThreadID  _threadId;       // the thread id of the queue
    protected int       _add_count = 0;  // the stack deepness
    protected boolean   _createThread;   // create a worker thread, if needed
    protected boolean   _createThread_now;   // create a worker thread, if needed
    protected Thread    _worker_thread;  // the thread that does the jobs

    protected Hashtable _disposeIds = new Hashtable();       // disposeIds for disposing

    protected JobQueue  _async_jobQueue; // chaining job qeueus for asyncs
    protected JobQueue  _sync_jobQueue;  // chaining job qeueus for syncs

    protected boolean _active = false;

    // statistics
    protected int _async_threads_created;
    protected int _sync_threads_created;
    protected int _async_jobs_queued;
    protected int _sync_jobs_queued;

    class MutableInt {
        int _value;
    }

    /**
     * A thread for dispatching jobs
     */
    class JobDispatcher extends Thread {
        JobDispatcher() {
//              super("JobDispatcher - " + _threadId);

            if(DEBUG) System.err.println("JobQueue$JobDispatcher.<init>:" + _threadId);

            if(_sync_jobQueue == null)
                ++ _sync_threads_created;
            else
                ++ _sync_jobQueue._async_threads_created;
        }

        ThreadID getThreadId() {
            return _threadId;
        }

        public void run() {
            if(DEBUG) System.err.println("ThreadPool$JobDispatcher.run");
            try {
                enter(1000, null);
            }
            catch(java.lang.Exception exception) {
                System.err.println(getClass().getName() + " - exception occurred:" + exception);
                if(DEBUG) ;exception.printStackTrace();
            }

            // dispose the jobQueue
//              dispose();
            if(DEBUG) System.err.println("##### " + getClass().getName() + ".run - exit:" + _threadId);
        }
    }


    /**
     * Constructs a async job queue with the given thread id
     * which belongs to the given sync job queue.
     * <p>
     * @param threadId         the thread id
     * @param sync_jobQueue    the sync queue this async queue belongs to
     * @see                    com.sun.star.lib.uno.environments.remote.ThreadID
     */
    protected JobQueue(ThreadID threadId, JobQueue sync_jobQueue) {
        ++ __instances;

        // create a new async threadID
        _threadId = new ThreadID(threadId);
        _sync_jobQueue = sync_jobQueue;
        _createThread = true;
        _createThread_now = true;

        if(DEBUG) System.err.println("##### " + getClass().getName() + " - init:" +  _threadId);
    }

    /**
     * Constructs a sync job queue with the given thread id and the given thread.
     * <p>
     * @param threadId        the thread id
     * @param createThread    if true, the queue creates a worker thread if needed
     * @see             com.sun.star.lib.uno.environments.remote.ThreadID
     */
    JobQueue(ThreadID threadId, boolean createThread){
        ++ __instances;

        _threadId     = threadId;
        _createThread = createThread;
        _createThread_now = createThread;

        if(DEBUG) System.err.println("##### " + getClass().getName() + " - init:" +  _threadId + " " + createThread);
    }

    /**
     * Gives the currently dispatched job.
     * <p>
     * @return  the dispatched job
     * @see     com.sun.star.lib.uno.environments.remote.Job
     */
    Job getCurrentJob() {
        return _current;
    }

    /**
     * Gives the thread id of this queue
     * <p>
     * @return  the thread id
     * @see     com.sun.star.lib.uno.environments.remote.ThreadID
     */
    ThreadID getThreadId() {
        return _threadId;
    }

    /**
     * Gives the dispatcher thread
     * <p>
     * @return  the thread
     */
    Thread getThread() {
        return _worker_thread;
    }

    /**
     * Adds a dispose id.
     * <p>
     * @return the count of how often the id has already been added
     * @param  disposeId    the dispose id
     */
    private synchronized MutableInt addDisposeId(Object disposeId) {
        MutableInt disposeId_count = null;

        if(disposeId != null) {
            if(DEBUG) System.err.println("##### " + getClass().getName() +  " " + this +".addDisposeId:" + disposeId);

            disposeId_count = (MutableInt)_disposeIds.get(disposeId);
            if(disposeId_count == null) {
                disposeId_count = new MutableInt();
                _disposeIds.put(disposeId, disposeId_count);
            }

            ++ disposeId_count._value;

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".addDisposeId value:" + disposeId_count._value);

        }

        return disposeId_count;
    }

    /**
     * Removes a dispose id.
     * <p>
     * @param  disposeId        the dispose id
     * @param  disposeId_count
     */
    private synchronized void removeDisposeId(Object disposeId, MutableInt disposeId_count) {
        if(disposeId != null) {
            if(DEBUG) System.err.println("##### " + getClass().getName() + ".removeDisposeId:" + disposeId + " " + disposeId_count);

            if(disposeId_count == null)
                disposeId_count = (MutableInt)_disposeIds.get(disposeId);

            if(disposeId_count != null) {
            -- disposeId_count._value;
            if(disposeId_count._value <= 0)
                _disposeIds.remove(disposeId);
            }
            if(DEBUG) System.err.println("##### " + getClass().getName() + ".removeDisposeId value:" + disposeId_count._value);

        }

    }

    /**
     * Removes a job from the queue.
     * <p>
     * @return a job or null if timed out
     * @param  waitTime        the maximum amount of time to wait for a job
     */
    private synchronized Job removeJob(int waitTime) throws InterruptedException {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".removeJob:" + /*_todo + " " + */_head + " " + _threadId);

        // wait max. waitTime time for a job to enter the queue
        boolean waited = false;
        while(_head == null && waitTime >= 0 && !waited) {
            wait(waitTime);

            // signal that we have already waited once
              waited = true;
        }

        // if there is an async queue, wait for jobs to be done
        if(_async_jobQueue != null) {
            synchronized(_async_jobQueue) {
                while(_async_jobQueue._worker_thread != null && _async_jobQueue._active) {
//                  while(_async_jobQueue._worker_thread != null || _async_jobQueue._head != null) {
//                  while(_async_jobQueue._todo > 0) {
                    if(DEBUG) System.err.println("waiting for async:" + _async_jobQueue._head + " " +  _async_jobQueue._worker_thread);
                    _async_jobQueue.wait(10);
                }
            }
        }

        Job job = null;

        if(_head != null) {
            _current = _head;
            _head    = _head._next;

            if(_head == null)
                _tail = null;

            job = _current;
        }

        return job;
    }

    /**
     * Puts a job into the queue.
     * <p>
     * @param  job        the job
     * @param  disposeId  a dispose id
     */
    public void putJob(Job job, Object disposeId) {
        if(job.getOperation() == null || job.isSynchron()) { // if job is a reply or is sync
            // fill the sync queue (this)
            _putJob(job, disposeId);

            ++ _sync_jobs_queued;
        }
        else {
            synchronized(this) {
                // create the async JobQueue ?
                if(_async_jobQueue == null)
                    _async_jobQueue = new JobQueue(_threadId, this);

                // fill the async queue, async queue are intentionally not disposed
                _async_jobQueue._putJob(job, null);

                ++ _async_jobs_queued;
            }
        }
    }

    /**
     * Puts a job into the queue.
     * <p>
     * @param  job        the job
     * @param  disposeId  a dispose id
     */
    private synchronized void _putJob(Job job, Object disposeId) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".putJob todoes: " /*+ _todo */ + " job:" + job);

        // Hold the dispose id at the, to be able to remove the dispose id
        // once the job has been executed.
        job._disposeId = disposeId;
        addDisposeId(disposeId);

        if(_tail != null)
            _tail._next = job;
        else
            _head = job;

        _tail = job;

//          ++ _todo;

        if(_worker_thread == null && _createThread && _createThread_now) { // if there is no thread, which dispatches and if shall create one, create one
            _createThread_now = false;
            new JobDispatcher().start();
        }

        // always notify possible waiters
        notify();
    }

    /**
     * Enters the job queue.
     * <p>
     * @return the result of the final job (reply)
     * @param  disposeId  a dispose id
     */
    Object enter(Object disposeId) throws Exception {
        return enter(0, disposeId); // wait infinitly
    }

    /**
     * Enters the job queue.
     * <p>
     * @return the result of the final job (reply)
     * @param  waitTime   the maximum amount of time to wait for a job (0 means wait infinitly)
     * @param  disposeId  a dispose id
     */
    Object enter(int waitTime, Object disposeId) throws Exception {
        if(DEBUG) System.err.println("#####" + getClass().getName() + ".enter: " + _threadId);

        boolean quit = false;

        MutableInt disposeId_count = addDisposeId(disposeId);

        Object result = null;

        Thread hold_worker_thread = _worker_thread;
        _worker_thread = Thread.currentThread();

        while(!quit) {
            Job job = null;
            try {
                job = removeJob(waitTime);

                if(job != null) {
                    try {
                        _active = true;
                        result = job.execute();
                    }
                    finally {
                        _active = false;
                        removeDisposeId(job._disposeId, null);
                    }

                    if(job.isFinal()) {
                        job.dispose();

                        quit = true;
                    }

                    job = null;
                }
                else
                    quit = true;


            }
            finally { // ensure that this queue becomes disposed, if necessary
                if(DEBUG) System.err.println("##### " + getClass().getName() + ".enter leaving: " + _threadId + " " + _worker_thread + " " + hold_worker_thread + " " + result);

                synchronized(this) {
                    if(job != null || (quit && _head == null)) {
                        _worker_thread = hold_worker_thread;

                        _createThread_now = true;

                        removeDisposeId(disposeId, disposeId_count);

                        if(_sync_jobQueue != null)
                            notifyAll(); // notify waiters (e.g. this is an asyncQueue and there is a sync waiting)
                    }
                    else
                        quit = false;

                }
            }
        }

        return result;
    }

    /**
     * If the given disposeId is registered,
     * interrups the worker thread.
     * <p>
     * @param disposeId    the dispose id
     */
    synchronized void interrupt(Object disposeId) {
        MutableInt disposeId_count = (MutableInt)_disposeIds.get(disposeId);

        if(DEBUG) System.err.println("##### " + getClass().getName() + " " + this + ".interrupt:" + disposeId + " " + disposeId_count);

        if(disposeId_count != null && _worker_thread != null) { //
            _worker_thread.interrupt();
        }
    }

    /**
     * The finalizer decreases the instance count
     */
    public void finalize() {
        -- __instances;
    }

    /**
     * Prints statistics abourt the queue
     */
    void printStats() {
        System.err.println("threads created all: " + (_sync_threads_created + _async_threads_created)
                           + " asyncs: " + _async_threads_created
                           + " syncs: " + _sync_threads_created);
        System.err.println("jobs pub - all: " + (_async_jobs_queued + _sync_jobs_queued)
                           + " asyncs: " + _async_jobs_queued
                           + " syncs: " + _sync_jobs_queued);
    }

    /**
     * Clears the queue
     */
    synchronized void clear() {
        if(_head != null)
            System.err.println("JobQueue.dispose - jobs left");

        _head = _tail = _current = null;
          _worker_thread = null;

//          _threadId = null;
//          _async_jobQueue = null;

//          notify(); // wakes up all threads, which are waiting for jobs
    }
}

