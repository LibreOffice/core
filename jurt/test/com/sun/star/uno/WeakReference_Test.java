/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WeakReference_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:18:10 $
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
