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
        queue = new ReferenceQueue<Object>();
        ref = new PhantomReference<Object>(obj, queue);
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
        }
        new WaitThread(obj).start();
    }

    private Object obj;
    private final ReferenceQueue<Object> queue;
    private final PhantomReference<Object> ref;
}
