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

/**
 * This class implements a java thread pool.
 * <p>
 * @see         com.sun.star.uno.UnoRuntime
 * @see         com.sun.star.lib.uno.environments.remote.ThreadPool
 * @see         com.sun.star.lib.uno.environments.remote.IThreadPool
 * @see         com.sun.star.lib.uno.environments.remote.Job
 * @see         com.sun.star.lib.uno.environments.remote.JobQueue
 * @since       UDK1.0
 */
public class JavaThreadPool implements IThreadPool {
    /**
     * When set to true, enables various debugging output.
     */
    private static final boolean DEBUG = false;

    JavaThreadPoolFactory _javaThreadPoolFactory;

    JavaThreadPool(JavaThreadPoolFactory javaThreadPoolFactory) {
        _javaThreadPoolFactory = javaThreadPoolFactory;
    }

    public ThreadId getThreadId() {
        return JavaThreadPoolFactory.getThreadId();
    }

    public Object attach( ThreadId threadId )
    {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".attach - id:" + threadId);
        JobQueue jobQueue = _javaThreadPoolFactory.getJobQueue(threadId);
        if(jobQueue == null)
            jobQueue = new JobQueue(_javaThreadPoolFactory, threadId, false);

        // acquiring the jobQueue registers it at the ThreadPoolFactory
        jobQueue.acquire();
        return jobQueue;
    }

    public void attach() {
        attach( getThreadId() );
    }

    public void detach( Object handle, ThreadId id )
    {
        ((JobQueue)handle).release();
    }

    public void detach() {
        ThreadId threadId =  getThreadId();
        detach(_javaThreadPoolFactory.getJobQueue(threadId), threadId );
    }


    public Object enter( ) throws Throwable {
        ThreadId threadId = getThreadId();
        return enter( _javaThreadPoolFactory.getJobQueue( threadId ), threadId  );
    }

    public Object enter( Object handle, ThreadId threadId ) throws Throwable {
        return ((JobQueue)handle).enter(this);
    }

    public void putJob(Job job) {
        if (!job.isRequest() || job.isSynchronous()) {
            JobQueue jobQueue = _javaThreadPoolFactory.getJobQueue(job.getThreadId());

            // this has not be synchronized, cause
            // sync jobs can only come over one bridge
            // (cause the thread blocks on other side)
            if(jobQueue == null)
                jobQueue = new JobQueue(_javaThreadPoolFactory, job.getThreadId(), true);

            // put job acquires the queue and registers it at the ThreadPoolFactory
            jobQueue.putJob(job, this);
        }
        else {
            // this has to be synchronized, cause
            // async jobs of the same thread can come
            // over different bridges
            synchronized(_javaThreadPoolFactory) {
                JobQueue async_jobQueue = _javaThreadPoolFactory.getAsyncJobQueue(job.getThreadId());

                // ensure there is jobQueue
                if(async_jobQueue == null) // so, there is really no async queue
                    async_jobQueue = new JobQueue(_javaThreadPoolFactory, job.getThreadId());

                // put job acquires the queue and registers it at the ThreadPoolFactory
                async_jobQueue.putJob(job, this);
            }
        }
    }

    public void dispose(Throwable throwable) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".dispose:" + throwable);

        _javaThreadPoolFactory.dispose(this, throwable);
    }

    public void destroy() {
    }
}
