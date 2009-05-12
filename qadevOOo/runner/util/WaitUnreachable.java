/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WaitUnreachable.java,v $
 * $Revision: 1.5 $
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

package util;

import java.lang.ref.PhantomReference;
import java.lang.ref.ReferenceQueue;

/**
 * Wait until an object has become unreachable.
 *
 * <p>Instances of this class will typically be used as either:</p>
 * <pre>
 *   SomeType o = new SomeType(...);
 *   ... // use "o"
 *   WaitUnreachable u = new WaitUnreachable(o);
 *   o = null;
 *   u.waitUnreachable();
 * </pre>
 * <p>or:</p>
 * <pre>
 *   WaitUnreachable u = new WaitUnreachable(new SomeType(...));
 *   ... // use "(SomeType) u.get()"
 *   u.waitUnreachable();
 * </pre>
 */
public final class WaitUnreachable {
    /**
     * Creates a new waiter.
     *
     * @param obj the object on which to wait
     */
    public WaitUnreachable(Object obj) {
        this.obj = obj;
        queue = new ReferenceQueue();
        ref = new PhantomReference(obj, queue);
    }

    /**
     * Gets the object on which to wait.
     *
     * @return the object on which to wait, or <code>null</code> if
     * <code>waitUnreachable</code> has already been called
     */
    public synchronized Object get() {
        return obj;
    }

    /**
     * Starts waiting for the object to become unreachable.
     *
     * <p>This blocks the current thread until the object has become
     * unreachable.</p>
     *
     * <p>Actually, this method waits until the JVM has <em>detected</em> that
     * the object has become unreachable.  This is not deterministic, but this
     * methods makes a best effort to cause the JVM to eventually detect the
     * situation.  With a typical JVM, this should suffice.</p>
     */
    public void waitUnreachable() {
        synchronized (this) {
            obj = null;
        }
        System.out.println("waiting for gc");
        while (queue.poll() == null) {
            System.gc();
            System.runFinalization();
            byte[] dummy = new byte[1024];
        }
    }

    /**
     * Ensures that an object will be finalized as soon as possible.
     *
     * <p>This does not block the current thread.  Instead, a new thread is
     * spawned that busy waits for the given object to become unreachable.</p>
     *
     * <p>This method cannot guarantee that the given object is eventually
     * finalized, but it makes a best effort.  With a typical JVM, this should
     * suffice.</p>
     *
     * @param obj the object of which to ensure finalization
     */
    public static void ensureFinalization(final Object obj) {
        final class WaitThread extends Thread {
            public WaitThread(Object obj) {
                super("ensureFinalization");
                unreachable = new WaitUnreachable(obj);
            }

            public void run() {
                unreachable.waitUnreachable();
            }

            private final WaitUnreachable unreachable;
        };
        new WaitThread(obj).start();
    }

    private Object obj;
    private final ReferenceQueue queue;
    private final PhantomReference ref;
}
