/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Cache_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:14:37 $
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
