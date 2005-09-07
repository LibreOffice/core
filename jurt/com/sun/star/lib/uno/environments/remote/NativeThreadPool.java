/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NativeThreadPool.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:01:26 $
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
        putJob(pool, job.getThreadId().getBytes(), job,
               job.getOperation() != null, !job.isSynchron());
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
