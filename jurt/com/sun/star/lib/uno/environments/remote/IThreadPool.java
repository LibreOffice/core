/*************************************************************************
 *
 *  $RCSfile: IThreadPool.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-09 10:09:56 $
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

/**
 * This interface is an abstraction of the various
 * threadpool implementations.
 * <p>
 * @version     $Revision: 1.5 $ $ $Date: 2003-10-09 10:09:56 $
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
     * all threads by throwing the given
     * <code>Throwable</code>.
     * <p>
     * @param throwing   the Throwable
     */
    public void dispose(Throwable throwable);


    /**
     * Destroys the thread pool and tries
     * to join all created threads immediatly.
     */
    public void destroy();
}

