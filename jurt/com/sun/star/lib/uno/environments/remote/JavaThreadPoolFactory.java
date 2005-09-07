/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JavaThreadPoolFactory.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:00:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package com.sun.star.lib.uno.environments.remote;

import java.util.Collection;
import java.util.HashMap;
import java.util.WeakHashMap;

final class JavaThreadPoolFactory {
    public JavaThreadPoolFactory() {}

    public IThreadPool createThreadPool() {
        return new JavaThreadPool(this);
    }

    public void addJobQueue(JobQueue jobQueue) {
        synchronized (jobQueues) {
            jobQueues.put(jobQueue.getThreadId(), jobQueue);
        }
    }

    public void removeJobQueue(JobQueue jobQueue) {
        synchronized (jobQueues) {
            jobQueues.remove(jobQueue.getThreadId());
        }
    }

    public JobQueue getJobQueue(ThreadId threadId) {
        synchronized (jobQueues) {
            return (JobQueue) jobQueues.get(threadId);
        }
    }

    public JobQueue getAsyncJobQueue(ThreadId threadId) {
        JobQueue q = getJobQueue(threadId);
        return q == null ? null : q._async_jobQueue;
    }

    public void dispose(Object disposeId, Throwable throwable) {
        JobQueue[] qs;
        synchronized (jobQueues) {
            Collection c = jobQueues.values();
            qs = (JobQueue[]) c.toArray(new JobQueue[c.size()]);
        }
        for (int i = 0; i < qs.length; ++i) {
            qs[i].dispose(disposeId, throwable);
        }
    }

    public static ThreadId getThreadId() {
        Thread t = Thread.currentThread();
        if (t instanceof JobQueue.JobDispatcher) {
            return ((JobQueue.JobDispatcher) t).getThreadId();
        } else {
            ThreadId id;
            synchronized (threadIdMap) {
                id = (ThreadId) threadIdMap.get(t);
                if (id == null) {
                    id = ThreadId.createFresh();
                    threadIdMap.put(t, id);
                }
            }
            return id;
        }
    }

    private static final WeakHashMap threadIdMap = new WeakHashMap();
    private final HashMap jobQueues = new HashMap();
}
