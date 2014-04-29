/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package com.sun.star.lib.uno.environments.remote;

/**
 * This interface is an abstraction of the various
 * threadpool implementations.
 * <p>
 * @version     $Revision: 1.7 $ $ $Date: 2008-04-11 11:20:01 $
 * @author      Joerg Budischewski
 * @author      Kay Ramme
 * @see         com.sun.star.lib.uno.environments.remote.ThreadPoolFactory
 * @see         com.sun.star.lib.uno.environments.remote.IThreadPoolFactory
 * @since       UDK1.0
 */
public interface IThreadPool {
    /**
     * Retrieves the global threadId for the current thread.
     * <p>
     * @return the thread id
     */
    ThreadId getThreadId();

    /**
     * Attaches this thread to the thread pool.
     * <p>
     * @see                 #enter
     */
    public void attach();

    /**
     * As above, but hands in an already existing
     * instance of the threadid of the current thread.
     * Returns a handle which can be used in enter and
     * detach calls.<p>
     * The function exists for performance
     * optimization reasons.
     * @see #attach
     */
    public Object attach( ThreadId id );

    /**
     * Detaches this thread from the thread pool.
     * @see                 #enter
     */
    public void detach();

    /**
     * As above, but hands in an already existing
     * instance of the threadid of the current thread
     * and a handle returned by attach.
     * The function exists for performance
     * optimization reasons.
     * @see #attach,#detach
     */
    public void detach( Object handle, ThreadId id );

    /**
     * Lets this thread enter the thread pool.
     * This thread then executes all jobs put via
     * <code>putJob</code> until a reply job arrives.
     * <p>
     * @see                 #putJob
     */
    public Object enter() throws Throwable;

    /**
     * as above but hands in an already existing
     * instance of the threadid of the current thread
     * and a handle returned by attach.
     * This thread then executes all jobs put via
     * <code>putJob</code> until a reply job arrives.
     * <p>
     * @see                 #putJob
     */
    public Object enter( Object handle, ThreadId id ) throws Throwable;

    /**
     * Queues a job into the jobQueue of the thread belonging
     * to the jobs threadId.
     * <p>
     * @param job       the job
     */
    public void putJob(Job job);

    /**
     * Disposes this thread pool, thus releasing
     * all threads by throwing a <code>DisposedException</code> with the given
     * <code>Throwable</code> cause.
     * <p>
     * @param throwing   the cause
     */
    public void dispose(Throwable throwable);


    /**
     * Destroys the thread pool and tries
     * to join all created threads immediately.
     */
    public void destroy();
}

