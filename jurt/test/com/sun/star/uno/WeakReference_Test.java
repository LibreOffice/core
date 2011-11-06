/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
