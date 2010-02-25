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

import complexlib.ComplexTestCase;
import util.WaitUnreachable;

public final class WeakMap_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() {
        WeakMap m = new WeakMap();
        assure("", m.size() == 0);
        assure("", m.isEmpty());
        assure("", !m.containsKey("key1"));
        assure("", !m.containsValue(null));
        WaitUnreachable u1 = new WaitUnreachable(new Object());
        m.put("key1", u1.get());
        WaitUnreachable u2 = new WaitUnreachable(new Disposable());
        m.put("key2", u2.get());
        assure("", m.size() == 2);
        assure("", !m.isEmpty());
        assure("", m.containsKey("key1"));
        assure("", m.containsKey("key2"));
        assure("", !m.containsKey("key3"));
        assure("", m.containsValue(m.get("key1")));
        assure("", m.containsValue(m.get("key2")));
        assure("", WeakMap.getValue(m.get("key1")).equals(u1.get()));
        assure("", WeakMap.getValue(m.get("key2")).equals(u2.get()));
        assure("", m.values().size() == 2);
        assure("", m.values().contains(m.get("key1")));
        assure("", m.values().contains(m.get("key2")));
        u1.waitUnreachable();
        assure("", WeakMap.getValue(m.get("key1")) == null);
        ((Disposable) u2.get()).dispose();
        assure("", WeakMap.getValue(m.get("key2")) == null);
        m.clear();
        u2.waitUnreachable();
        assure("", m.size() == 0);
        m.put("key2", new Object());
        assure("", m.size() == 1);
    }

    // This simple class (single listener, no synchronization) exploits
    // knowledge about the implementation of WeakMap:
    private static final class Disposable implements DisposeNotifier {
        public void addDisposeListener(DisposeListener listener) {
            this.listener = listener;
        }

        public void dispose() {
            if (listener != null) {
                listener.notifyDispose(this);
            }
        }

        private DisposeListener listener = null;
    }
}
