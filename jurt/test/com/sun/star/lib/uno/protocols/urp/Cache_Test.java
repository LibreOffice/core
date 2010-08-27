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

import complexlib.ComplexTestCase;

public final class Cache_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test0", "test1", "test2", "test3" };
    }

    public void test0() {
        Cache c = new Cache(0);
        boolean[] f = new boolean[1];
        int i;
        i = c.add(f, "a");
        assure("1", i == Cache.NOT_CACHED && !f[0]);
        i = c.add(f, "a");
        assure("2", i == Cache.NOT_CACHED && !f[0]);
        i = c.add(f, "b");
        assure("3", i == Cache.NOT_CACHED && !f[0]);
        i = c.add(f, "a");
        assure("4", i == Cache.NOT_CACHED && !f[0]);
    }

    public void test1() {
        Cache c = new Cache(1);
        boolean[] f = new boolean[1];
        int i;
        i = c.add(f, "a");
        assure("1", i == 0 && !f[0]);
        i = c.add(f, "a");
        assure("2", i == 0 && f[0]);
        i = c.add(f, "b");
        assure("3", i == 0 && !f[0]);
        i = c.add(f, "b");
        assure("4", i == 0 && f[0]);
        i = c.add(f, "a");
        assure("5", i == 0 && !f[0]);
    }

    public void test2() {
        Cache c = new Cache(2);
        boolean[] f = new boolean[1];
        int i;
        i = c.add(f, "a");
        assure("1", i == 0 && !f[0]);
        i = c.add(f, "a");
        assure("2", i == 0 && f[0]);
        i = c.add(f, "b");
        assure("3", i == 1 && !f[0]);
        i = c.add(f, "b");
        assure("4", i == 1 && f[0]);
        i = c.add(f, "a");
        assure("5", i == 0 && f[0]);
        i = c.add(f, "c");
        assure("6", i == 1 && !f[0]);
        i = c.add(f, "b");
        assure("7", i == 0 && !f[0]);
    }

    public void test3() {
        Cache c = new Cache(3);
        boolean[] f = new boolean[1];
        int i;
        i = c.add(f, "a");
        assure("1", i == 0 && !f[0]);
        i = c.add(f, "a");
        assure("3", i == 0 && f[0]);
        i = c.add(f, "b");
        assure("5", i == 1 && !f[0]);
        i = c.add(f, "a");
        assure("3", i == 0 && f[0]);
        i = c.add(f, "c");
        assure("7", i == 2 && !f[0]);
        i = c.add(f, "d");
        assure("9", i == 1 && !f[0]);
        i = c.add(f, "d");
        assure("11", i == 1 && f[0]);
    }
}
