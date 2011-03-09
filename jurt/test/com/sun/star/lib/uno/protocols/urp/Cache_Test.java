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
