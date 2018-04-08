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
