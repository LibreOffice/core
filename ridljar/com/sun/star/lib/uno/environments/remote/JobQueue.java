/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import java.util.ArrayList;

import com.sun.star.lang.DisposedException;

/**
 * The <code>JobQueue</code> implements a queue for jobs.
 *
 * <p>For every jobs thread id exists a job queue which is registered
 * at the <code>ThreadPool</code>.</p>
 *
 * <p>A JobQueue is split into a sync job queue and an async job queue.
 * The sync job queue is the registered queue, it delegates async jobs
 * (put by <code>putjob</code>) into the async queue, which is only
 * known by the sync queue.</p>
 *
 * @see         com.sun.star.lib.uno.environments.remote.IThreadPool
 * @see         com.sun.star.lib.uno.environments.remote.Job
 * @see         com.sun.star.lib.uno.environments.remote.ThreadId
 * @since       UDK1.0
 */
public class JobQueue {
    /**
     * When set to true, enables various debugging output.
     */
    private static final boolean DEBUG = false;

    final ArrayList<Job> jobList = new ArrayList<Job>();

    private ThreadId  _threadId;       // the thread id of the queue
    protected int     _ref_count = 0;  // the stack deepness
    private boolean   _createThread;   // create a worker thread, if needed
    private boolean   _createThread_now;   // create a worker thread, if needed
    private Thread    _worker_thread;  // the thread that does the jobs

    private Object    _disposeId; // the active dispose id
    private Object    _doDispose = null;
    private Throwable _throwable;

    JobQueue  _async_jobQueue; // chaining job queues for asyncs
    protected JobQueue  _sync_jobQueue;  // chaining job queues for syncs

    private boolean _active = false;

    private JavaThreadPoolFactory _javaThreadPoolFactory;

    /**
     * A thread for dispatching jobs.
     */
    class JobDispatcher extends Thread {
        Object _disposeId;

        JobDispatcher(Object disposeId) {
            super("JobDispatcher");

            if(DEBUG) System.err.println("JobQueue$JobDispatcher.<init>:" + _threadId);

            _disposeId = disposeId;
        }

        ThreadId getThreadId() {
            return _threadId;
        }

        @Override
        public void run() {
            if(DEBUG) System.err.println("ThreadPool$JobDispatcher.run: " + Thread.currentThread());

            try {
                  enter(2000, _disposeId);
            } catch(Throwable throwable) {
                synchronized (JobQueue.this) {
                    if(!jobList.isEmpty() || _active) { // there was a job in progress, so give a stack
                        System.err.println(getClass().getName() + " - exception occurred:" + throwable);
                        throwable.printStackTrace(System.err);
                    }
                }
            }
            finally {
                release();
            }

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".run - exit:" + _threadId);
        }
    }


    /**
     * Constructs an async job queue with the given thread id which belongs to
     * the given sync job queue.
     *
     * @param threadId         the thread id.
     * @see                    com.sun.star.lib.uno.environments.remote.ThreadId
     */
    JobQueue(JavaThreadPoolFactory javaThreadPoolFactory, ThreadId threadId) {
        _javaThreadPoolFactory = javaThreadPoolFactory;
        _threadId = ThreadId.createFresh();

        _sync_jobQueue    = javaThreadPoolFactory.getJobQueue(threadId);
        if(_sync_jobQueue == null) {
            _sync_jobQueue = new JobQueue(javaThreadPoolFactory, threadId, true);
            _sync_jobQueue.acquire();
        }

        _sync_jobQueue._async_jobQueue = this;

        _createThread     = true;
        _createThread_now = true;

        acquire();

        if(DEBUG) System.err.println("##### " + getClass().getName() + " - init:" +  _threadId);
    }

    /**
     * Constructs a sync job queue with the given thread id and the given thread.
     *
     * @param threadId        the thread id.
     * @param createThread    if true, the queue creates a worker thread if needed.
     * @see             com.sun.star.lib.uno.environments.remote.ThreadId
     */
    JobQueue(JavaThreadPoolFactory javaThreadPoolFactory, ThreadId threadId, boolean createThread){
        _javaThreadPoolFactory   = javaThreadPoolFactory;
        _threadId         = threadId;
        _createThread     = createThread;
        _createThread_now = createThread;

        if(DEBUG) System.err.println("##### " + getClass().getName() + " - init:" +  _threadId + " " + createThread);
    }

    /**
     * Gives the thread id of this queue.
     *
     * @return  the thread id.
     * @see     com.sun.star.lib.uno.environments.remote.ThreadId
     */
    ThreadId getThreadId() {
        return _threadId;
    }

    synchronized void acquire() {
        // add only synchronous queues .
        if(_ref_count <= 0 && _sync_jobQueue == null )
            _javaThreadPoolFactory.addJobQueue(this);

        ++ _ref_count;
    }

    synchronized void release() {
        -- _ref_count;

        if(_ref_count <= 0) {
            // only synchronous queues needs to be removed .
            if( _sync_jobQueue == null )
                _javaThreadPoolFactory.removeJobQueue(this);


            if(_sync_jobQueue != null) {
                _sync_jobQueue._async_jobQueue = null;
                _sync_jobQueue.release();
            }
        }
    }

    /**
     * Removes a job from the queue.
     *
     * @param  waitTime the maximum amount of time to wait for a job.
     * @return a job or null if timed out.
     */
    private Job removeJob(int waitTime) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".removeJob:" + jobList + " " + _threadId);

        Job job = null;
        synchronized (this) {
            // wait max. waitTime time for a job to enter the queue
            boolean waited = false;
            while(jobList.isEmpty() && (waitTime == 0 || !waited)) {
                if(_doDispose == _disposeId) {
                    _doDispose = null;
                    throw (DisposedException)
                        new DisposedException().initCause(_throwable);
                }

                // notify sync queues
                notifyAll();

                try {
                    // wait for new job
                    wait(waitTime);
                } catch(InterruptedException interruptedException) {
                      throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".removeJob - unexpected:" + interruptedException);
                }

                // signal that we have already waited once
                waited = true;
            }


            if(!jobList.isEmpty()) {
                job = jobList.remove(0);
                _active = true;
            }
        }

        // always wait for asynchron jobqueue to be finished !
        if(job != null && _async_jobQueue != null) {
            synchronized(_async_jobQueue) {
                // wait for async queue to be empty and last job to be done
                while(_async_jobQueue._active || !_async_jobQueue.jobList.isEmpty()) {
                    if(DEBUG) System.err.println("waiting for async:" + _async_jobQueue.jobList + " " +  _async_jobQueue._worker_thread);

                    if(_doDispose == _disposeId) {
                        _doDispose = null;
                        throw (DisposedException)
                            new DisposedException().initCause(_throwable);
                    }

                    try {
                        _async_jobQueue.wait();
                    } catch(InterruptedException interruptedException) {
                        throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".removeJob - unexpected:" + interruptedException);
                    }
                }
            }
        }

        return job;
    }

    /**
     * Puts a job into the queue.
     *
     * @param  job        the job.
     * @param  disposeId  a dispose id.
     */
    synchronized void putJob(Job job, Object disposeId) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".putJob todoes: " + " job:" + job);

        jobList.add(job);

        if(_worker_thread == null && _createThread && _createThread_now) { // if there is no thread, which dispatches and if shall create one, create one

            acquire();

            _createThread_now = false;
            new JobDispatcher(disposeId).start();
        }

        // always notify possible waiters
        notifyAll();
    }

    /**
     * Enters the job queue.
     *
     * @param  disposeId  a dispose id.
     * @return the result of the final job (reply).
     */
    Object enter(Object disposeId) throws Throwable {
        return enter(0, disposeId); // wait infinitely
    }

    /**
     * Enters the job queue.
     *
     * @param  waitTime   the maximum amount of time to wait for a job (0 means wait infinitely).
     * @param  disposeId  a dispose id.
     * @return the result of the final job (reply).
     */
    Object enter(int waitTime, Object disposeId) throws Throwable {
        if(DEBUG) System.err.println("#####" + getClass().getName() + ".enter: " + _threadId);

        boolean quit = false;

        Object hold_disposeId = _disposeId;
        _disposeId = disposeId;

        Object result = null;

        Thread hold_worker_thread = _worker_thread;
        _worker_thread = Thread.currentThread();

        while(!quit) {
            Job job = null;

            try {
                job = removeJob(waitTime);

                if(job != null) {
                    try {
                        result = job.execute();
                    }
                    finally {
                        _active = false;
                    }

                    if (!job.isRequest()) {
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
                    if(job != null || (quit && jobList.isEmpty())) {
                        _worker_thread = hold_worker_thread;

                        _createThread_now = true;

                        _disposeId = hold_disposeId;

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
     * If the given disposeId is registered, interrupts the worker thread.
     *
     * @param disposeId    the dispose id.
     */
    synchronized void dispose(Object disposeId, Throwable throwable) {
        if(_sync_jobQueue == null) { // dispose only sync queues
            _doDispose = disposeId;
            _throwable = throwable;

            // get thread out of wait and let it throw the throwable
            if(DEBUG) System.err.println(getClass().getName() + ".dispose - notifying thread");

            notifyAll();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
