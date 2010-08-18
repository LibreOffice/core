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
