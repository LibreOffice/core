/*************************************************************************
 *
 *  $RCSfile: JavaThreadPool.java,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-09 10:10:15 $
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


import java.util.Enumeration;
import java.util.Hashtable;


import com.sun.star.uno.UnoRuntime;

/**
 * This class implements a java thread pool.
 * <p>
 * @version     $Revision: 1.10 $ $ $Date: 2003-10-09 10:10:15 $
 * @author      Kay Ramme
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
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".putJob:" + job.isSynchron() + " " + job.getThreadId());

        if(job.isSynchron() || job.getOperation() == null) { // note: replys must be synchron
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
