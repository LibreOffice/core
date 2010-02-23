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

package com.sun.star.lib.util;

import java.util.LinkedList;

/**
   Helper class to asynchronously execute finalize methods.

   Current JVMs seem not to be robust against long-running finalize methods, in
   that such long-running finalize methods may lead to OutOfMemoryErrors.  This
   class mitigates the problem by asynchronously shifting the bodies of
   potentially long-running finalize methods into an extra thread.  Classes that
   make use of this in their finalize methods are the proxies used in the
   intra-process JNI UNO bridge and the inter-process Java URP UNO bridge (where
   in both cases finalizers lead to synchronous UNO release calls).

   If JVMs are getting more mature and should no longer have problems with
   long-running finalize mehtods, this class could be removed again.
*/
public final class AsynchronousFinalizer {
    /**
       Add a job to be executed asynchronously.

       The run method of the given job is called exactly once.  If it terminates
       abnormally by throwing any Throwable, that is ignored.

       @param job represents the body of some finalize method; must not be null.
    */
    public static void add(Job job) {
        synchronized (queue) {
            boolean first = queue.isEmpty();
            queue.add(job);
            if (first) {
                queue.notify();
            }
        }
    }

    /**
       An interface to represent bodies of finalize methods.

       Similar to Runnable, except that the run method may throw any Throwable
       (which is effectively ignored by AsynchronousFinalizer.add, similar to
       any Throwables raised by finalize being ignored).
    */
    public interface Job {
        void run() throws Throwable;
    }

    private static final LinkedList queue = new LinkedList();

    static {
        Thread t = new Thread() {
                public void run() {
                    for (;;) {
                        Job j;
                        synchronized (queue) {
                            while (queue.isEmpty()) {
                                try {
                                    queue.wait();
                                } catch (InterruptedException e) {}
                            }
                            j = (Job) queue.remove(0);
                        }
                        try {
                            j.run();
                        } catch (Throwable e) {}
                    }
                }
            };
        t.setDaemon(true);
        t.start();
    }

    private AsynchronousFinalizer() {}
}
