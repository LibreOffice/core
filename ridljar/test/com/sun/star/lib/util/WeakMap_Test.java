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

import org.junit.Test;
import util.WaitUnreachable;
import static org.junit.Assert.*;

public final class WeakMap_Test {
    @Test public void test() {
        WeakMap m = new WeakMap();
        assertEquals(0, m.size());
        assertTrue(m.isEmpty());
        assertFalse(m.containsKey("key1"));
        assertFalse(m.containsValue(null));
        WaitUnreachable u1 = new WaitUnreachable(new Object());
        m.put("key1", u1.get());
        WaitUnreachable u2 = new WaitUnreachable(new Disposable());
        m.put("key2", u2.get());
        assertEquals(2, m.size());
        assertFalse(m.isEmpty());
        assertTrue(m.containsKey("key1"));
        assertTrue(m.containsKey("key2"));
        assertFalse(m.containsKey("key3"));
        assertTrue(m.containsValue(m.get("key1")));
        assertTrue(m.containsValue(m.get("key2")));
        assertEquals(u1.get(), WeakMap.getValue(m.get("key1")));
        assertEquals(u2.get(), WeakMap.getValue(m.get("key2")));
        assertEquals(2, m.values().size());
        assertTrue(m.values().contains(m.get("key1")));
        assertTrue(m.values().contains(m.get("key2")));
        u1.waitUnreachable();
        assertNull(WeakMap.getValue(m.get("key1")));
        ((Disposable) u2.get()).dispose();
        assertNull(WeakMap.getValue(m.get("key2")));
        m.clear();
        u2.waitUnreachable();
        assertEquals(0, m.size());
        m.put("key2", new Object());
        assertEquals(1, m.size());
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
