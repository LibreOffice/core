/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AsynchronousFinalizer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-01-09 09:48:43 $
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
