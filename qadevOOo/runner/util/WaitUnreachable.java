/*************************************************************************
 *
 *  $RCSfile: WaitUnreachable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-13 17:20:05 $
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
