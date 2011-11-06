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

final class NativeThreadPool implements IThreadPool {
    public NativeThreadPool() {
        pool = create();
    }

    public ThreadId getThreadId() {
        return new ThreadId(threadId());
    }

    public void attach() {
        attach(pool);
    }

    public Object attach(ThreadId id) {
        attach();
        return null;
    }

    public void detach() {
        detach(pool);
    }

    public void detach(Object handle, ThreadId id) {
        detach();
    }

    public Object enter() throws Throwable {
        Job job = enter(pool);
        if (job == null) {
            throw dispose;
        }
        return job.execute();
    }

    public Object enter(Object handle, ThreadId id) throws Throwable {
        return enter();
    }

    public void putJob(Job job) {
        putJob(
            pool, job.getThreadId().getBytes(), job, job.isRequest(),
            job.isRequest() && !job.isSynchronous());
    }

    public void dispose(Throwable throwable) {
        dispose = throwable;
        dispose(pool);
    }

    public void destroy() {
        destroy(pool);
    }

    // The native implementation is in
    // bridges/source/jni_uno/nativethreadpool.cxx:
    static {
        System.loadLibrary("java_uno");
    }
    private static native byte[] threadId();
    private static native long create();
    private static native void attach(long pool);
    private static native Job enter(long pool);
    private static native void detach(long pool);
    private static native void putJob(
        long pool, byte[] threadId, Job job, boolean request, boolean oneWay);
    private static native void dispose(long pool);
    private static native void destroy(long pool);

    private final long pool;
    private volatile Throwable dispose;
}
