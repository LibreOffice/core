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

import java.util.Collection;
import java.util.HashMap;
import java.util.WeakHashMap;

final class JavaThreadPoolFactory {

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
            return jobQueues.get(threadId);
        }
    }

    public JobQueue getAsyncJobQueue(ThreadId threadId) {
        JobQueue q = getJobQueue(threadId);
        return q == null ? null : q._async_jobQueue;
    }

    public void notifyAboutSomeDisposedPool() {
        JobQueue[] qs;
        synchronized (jobQueues) {
            Collection<JobQueue> c = jobQueues.values();
            qs = c.toArray(new JobQueue[c.size()]);
        }
        for (int i = 0; i < qs.length; ++i) {
            qs[i].notifyAboutSomeDisposedPool();
        }
    }

    public static ThreadId getThreadId() {
        Thread t = Thread.currentThread();
        if (t instanceof JobQueue.JobDispatcher) {
            return ((JobQueue.JobDispatcher) t).getThreadId();
        } else {
            ThreadId id;
            synchronized (threadIdMap) {
                id = threadIdMap.get(t);
                if (id == null) {
                    id = ThreadId.createFresh();
                    threadIdMap.put(t, id);
                }
            }
            return id;
        }
    }

    private static final WeakHashMap<Thread, ThreadId> threadIdMap = new WeakHashMap<Thread, ThreadId>();
    private final HashMap<ThreadId, JobQueue> jobQueues = new HashMap<ThreadId, JobQueue>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
