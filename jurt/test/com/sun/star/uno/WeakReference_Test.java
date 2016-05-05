/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.uno;

import java.util.ArrayList;
import java.util.Iterator;
import org.junit.Test;
import util.WaitUnreachable;
import static org.junit.Assert.*;

public final class WeakReference_Test {
    @Test public void test() {
        Object o = new MockWeak();
        WeakReference r1 = new WeakReference(o);
        WeakReference r2 = new WeakReference(r1);
        assertSame(o, r1.get());
        assertSame(o, r2.get());
        WaitUnreachable u = new WaitUnreachable(o);
        o = null;
        u.waitUnreachable();
        assertNull("a3", r1.get());
        assertNull("a4", r2.get());
    }

    private static final class MockWeak implements XWeak {
        public XAdapter queryAdapter() {
            return adapter;
        }

        @Override
        protected void finalize() throws Throwable {
            adapter.dispose();
            super.finalize();
        }

        private static final class Adapter implements XAdapter {
            public Adapter(Object obj) {
                ref = new java.lang.ref.WeakReference<Object>(obj);
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
                ArrayList<XReference> l;
                synchronized (this){
                    l = listeners;
                    listeners = null;
                }
                if (l != null) {
                    java.lang.RuntimeException ex = null;
                    for (Iterator<XReference> i = l.iterator(); i.hasNext();) {
                        try {
                            i.next().dispose();
                        } catch (java.lang.RuntimeException e) {
                            ex = e;
                        }
                    }
                    if (ex != null) {
                        throw ex;
                    }
                }
            }

            private final java.lang.ref.WeakReference<Object> ref;
            private ArrayList<XReference> listeners = new ArrayList<XReference>();
        }

        private final Adapter adapter = new Adapter(this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
