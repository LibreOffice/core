/*************************************************************************
 *
 *  $RCSfile: NativeThreadPool.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-09 10:10:36 $
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
