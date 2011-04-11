/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
     * to join all created threads immediatly.
     */
    public void destroy();
}

