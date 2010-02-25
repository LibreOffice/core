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
