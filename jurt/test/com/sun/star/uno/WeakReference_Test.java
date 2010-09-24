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

package com.sun.star.uno;

import complexlib.ComplexTestCase;
import java.util.ArrayList;
import java.util.Iterator;
import util.WaitUnreachable;

public final class WeakReference_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() {
        Object o = new MockWeak();
        WeakReference r1 = new WeakReference(o);
        WeakReference r2 = new WeakReference(r1);
        assure("", r1.get() == o);
        assure("", r2.get() == o);
        WaitUnreachable u = new WaitUnreachable(o);
        o = null;
        u.waitUnreachable();
        assure("a3", r1.get() == null);
        assure("a4", r2.get() == null);
    }

    private static final class MockWeak implements XWeak {
        public XAdapter queryAdapter() {
            return adapter;
        }

        protected void finalize() {
            adapter.dispose();
        }

        private static final class Adapter implements XAdapter {
            public Adapter(Object obj) {
                ref = new java.lang.ref.WeakReference(obj);
            }

            public Object queryAdapted() {
                return ref.get();
            }

            public void addReference(XReference ref) {
                synchronized (this) {
                    if (listeners != null) {
                        listeners.add(ref);
                        return;
                    }
                }
                ref.dispose();
            }

            public synchronized void removeReference(XReference ref) {
                if (listeners != null) {
                    listeners.remove(ref);
                }
            }

            public void dispose() {
                ArrayList l;
                synchronized (this){
                    l = listeners;
                    listeners = null;
                }
                if (l != null) {
                    java.lang.RuntimeException ex = null;
                    for (Iterator i = l.iterator(); i.hasNext();) {
                        try {
                            ((XReference) i.next()).dispose();
                        } catch (java.lang.RuntimeException e) {
                            ex = e;
                        }
                    }
                    if (ex != null) {
                        throw ex;
                    }
                }
            }

            private final java.lang.ref.WeakReference ref;
            private ArrayList listeners = new ArrayList();
        }

        private final Adapter adapter = new Adapter(this);
    }
}
