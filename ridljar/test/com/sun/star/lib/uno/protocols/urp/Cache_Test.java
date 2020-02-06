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

package com.sun.star.lib.uno.protocols.urp;

import org.junit.Test;
import static org.junit.Assert.*;

public final class Cache_Test {
    @Test public void test0() {
        Cache c = new Cache(0);
        boolean[] f = new boolean[1];
        int i;
        i = c.add(f, "a");
        assertTrue(i == Cache.NOT_CACHED && !f[0]);
        i = c.add(f, "a");
        assertTrue(i == Cache.NOT_CACHED && !f[0]);
        i = c.add(f, "b");
        assertTrue(i == Cache.NOT_CACHED && !f[0]);
        i = c.add(f, "a");
        assertTrue(i == Cache.NOT_CACHED && !f[0]);
    }

    @Test public void test1() {
        Cache c = new Cache(1);
        boolean[] f = new boolean[1];
        int i;
        i = c.add(f, "a");
        assertTrue(i == 0 && !f[0]);
        i = c.add(f, "a");
        assertTrue(i == 0 && f[0]);
        i = c.add(f, "b");
        assertTrue(i == 0 && !f[0]);
        i = c.add(f, "b");
        assertTrue(i == 0 && f[0]);
        i = c.add(f, "a");
        assertTrue(i == 0 && !f[0]);
    }

    @Test public void test2() {
        Cache c = new Cache(2);
        boolean[] f = new boolean[1];
        int i;
        i = c.add(f, "a");
        assertTrue(i == 0 && !f[0]);
        i = c.add(f, "a");
        assertTrue(i == 0 && f[0]);
        i = c.add(f, "b");
        assertTrue(i == 1 && !f[0]);
        i = c.add(f, "b");
        assertTrue(i == 1 && f[0]);
        i = c.add(f, "a");
        assertTrue(i == 0 && f[0]);
        i = c.add(f, "c");
        assertTrue(i == 1 && !f[0]);
        i = c.add(f, "b");
        assertTrue(i == 0 && !f[0]);
    }

    @Test public void test3() {
        Cache c = new Cache(3);
        boolean[] f = new boolean[1];
        int i;
        i = c.add(f, "a");
        assertTrue(i == 0 && !f[0]);
        i = c.add(f, "a");
        assertTrue(i == 0 && f[0]);
        i = c.add(f, "b");
        assertTrue(i == 1 && !f[0]);
        i = c.add(f, "a");
        assertTrue(i == 0 && f[0]);
        i = c.add(f, "c");
        assertTrue(i == 2 && !f[0]);
        i = c.add(f, "d");
        assertTrue(i == 1 && !f[0]);
        i = c.add(f, "d");
        assertTrue(i == 1 && f[0]);
    }

    @Test public void testNothingLostFromLruList() {
        // Regardless in what order arbitrary values from 0, ..., 3 are inserted
        // into a size-4 cache, afterwards adding -1, ..., -4 must return each
        // possible index in the range from 0, ..., 3 exactly once (so their sum
        // must be 6); this code systematically tests all such arbitrary ways up
        // to length 8 (the code arguably violates recommendations for writing
        // good tests, but actually helped track down an error in the Cache
        // implementation):
        int[] a = new int[8];
        for (int i = 0; i < a.length; ++i) {
            for (int j = 0; j < i; ++j) {
                a[j] = 0;
            }
            for (;;) {
                Cache c = new Cache(4);
                for (int k = 0; k < i; ++k) {
                    c.add(new boolean[1], a[k]);
                }
                assertEquals(
                    6,
                    (c.add(new boolean[1], -1) + c.add(new boolean[1], -2) +
                     c.add(new boolean[1], -3) + c.add(new boolean[1], -4)));
                int j = i - 1;
                while (j >= 0 && a[j] == 3) {
                    --j;
                }
                if (j < 0) {
                    break;
                }
                ++a[j];
                for (int k = j + 1; k < i; ++k) {
                    a[k] = 0;
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
