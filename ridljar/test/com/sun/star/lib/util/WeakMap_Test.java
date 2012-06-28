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

package com.sun.star.lib.util;

import org.junit.Test;
import util.WaitUnreachable;
import static org.junit.Assert.*;

public final class WeakMap_Test {
    @Test public void test() {
        WeakMap<String,Object> m = new WeakMap<String,Object>();
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
