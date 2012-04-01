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

package com.sun.star.uno;

import com.sun.star.beans.Optional;
import org.junit.Test;
import static org.junit.Assert.*;

public final class UnoRuntime_Test {
    @Test public void test_generateOid() {
        // Test if UnoRuntime generates an OID for a simple class:
        assertNotNull("Test1", UnoRuntime.generateOid(new Test1()));

        // Test if UnoRuntime generates an OID for a class implementing
        // IQueryInterface and returning null from getOid:
        assertNotNull("Test2", UnoRuntime.generateOid(new Test2()));

        // Test if a delegator object has the same OID as its creator:
        Test4 test4 = new Test4();
        Ifc ifc = UnoRuntime.queryInterface(Ifc.class, test4);
        assertEquals(
            "Test4", UnoRuntime.generateOid(ifc),
            UnoRuntime.generateOid(test4));
    }

    @Test public void test_queryInterface() {
        // Test if a query for an interface which is not supported returns null:
        assertNull("Test1", UnoRuntime.queryInterface(Ifc.class, new Test1()));

        // Test if a query for an interface which is supported through
        // IQueryInterface succeeds:
        assertNotNull(
            "Test2", UnoRuntime.queryInterface(Ifc.class, new Test2()));

        // Test if a query for an interface which is directly supported (through
        // inheritance) succeeds:
        assertNotNull(
            "Test3", UnoRuntime.queryInterface(Ifc.class, new Test3()));
    }

    @Test public void test_areSame() {
        assertTrue(
            UnoRuntime.areSame(
                new Any(Type.UNSIGNED_LONG, new Integer(3)),
                new Any(Type.UNSIGNED_LONG, new Integer(3))));
        assertFalse(
            UnoRuntime.areSame(
                new Any(Type.UNSIGNED_LONG, new Integer(3)), new Integer(3)));
        assertFalse(UnoRuntime.areSame(new int[] { 1 }, new int[] { 1, 2 }));
        assertTrue(
            UnoRuntime.areSame(
                TypeClass.UNSIGNED_LONG,
                new Any(new Type(TypeClass.class), TypeClass.UNSIGNED_LONG)));
        assertTrue(
            UnoRuntime.areSame(
                new Any(
                    new Type("com.sun.star.beans.Optional<unsigned long>"),
                    new Optional()),
                new Any(
                    new Type("com.sun.star.beans.Optional<unsigned long>"),
                    new Optional(false, new Integer(0)))));
        assertFalse(UnoRuntime.areSame(new Test1(), new Test2()));
        Test2 test2 = new Test2();
        assertTrue(
            "Test2",
            UnoRuntime.areSame(
                UnoRuntime.queryInterface(Ifc.class, test2), test2));
    }

    @Test public void test_completeValue() {
        assertEquals(
            new Integer(0), UnoRuntime.completeValue(Type.UNSIGNED_LONG, null));
        Object v = UnoRuntime.completeValue(
            new Type("[][]unsigned long"), null);
        assertTrue(v instanceof int[][]);
        assertEquals(0, ((int[][]) v).length);
        assertSame(
            TypeClass.VOID,
            UnoRuntime.completeValue(new Type(TypeClass.class), null));
        v = UnoRuntime.completeValue(
            new Type("com.sun.star.beans.Optional<unsigned long>"), null);
        assertTrue(v instanceof Optional);
        assertFalse(((Optional) v).IsPresent);
        assertNull(((Optional) v).Value);
    }

    @Test public void test_currentContext() throws InterruptedException {
        TestThread t1 = new TestThread();
        TestThread t2 = new TestThread();
        t1.start();
        t2.start();
        t1.join();
        t2.join();
        Object v1 = t1.context.getValueByName("");
        Object v2 = t2.context.getValueByName("");
        assertFalse(t1.context == t2.context);
        assertTrue(v1 == t1);
        assertTrue(v2 == t2);
        assertFalse(v1 == v2);
    }

    private interface Ifc extends XInterface {}

    private static class Test1 {}

    private static class Test2 implements XInterface, IQueryInterface {
        public String getOid() {
            return null;
        }

        public Object queryInterface(Type type) {
            return type.equals(new Type(Ifc.class)) ? t2 : null;
        }

        public boolean isSame(Object object) {
            return object == t2;
        }

        private static final class T2 implements Ifc {}

        private final T2 t2 = new T2();
    }

    private static class Test3 implements Ifc {}

    private static class Test4 implements XInterface, IQueryInterface {
        public String getOid() {
            return null;
        }

        public Object queryInterface(Type type) {
            return type.equals(new Type(Ifc.class)) ? t4 : null;
        }

        public boolean isSame(Object object) {
            return object == t4;
        }

        private final class T4 implements Ifc, IQueryInterface {
            public String getOid() {
                return UnoRuntime.generateOid(Test4.this);
            }

            public Object queryInterface(Type type) {
                return Test4.this.queryInterface(type);
            }

            public boolean isSame(Object object) {
                return UnoRuntime.areSame(Test4.this, object);
            }
        }

        private final T4 t4 = new T4();
    }

    private final class TestThread extends Thread {
        public void run() {
            //TODO: JUnit does not notice if these asserts fail:
            assertNull(UnoRuntime.getCurrentContext());
            context = new TestCurrentContext();
            UnoRuntime.setCurrentContext(context);
            assertSame(context, UnoRuntime.getCurrentContext());
            assertSame(this, context.getValueByName(""));
            UnoRuntime.setCurrentContext(null);
            assertNull(UnoRuntime.getCurrentContext());
        }

        public XCurrentContext context = null;
    }

    private static final class TestCurrentContext implements XCurrentContext {
        public Object getValueByName(String name) {
            return value;
        }

        private final Object value = Thread.currentThread();
    }
}
