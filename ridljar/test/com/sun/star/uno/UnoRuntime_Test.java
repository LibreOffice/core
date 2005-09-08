/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UnoRuntime_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:28:18 $
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

package com.sun.star.uno;

import complexlib.ComplexTestCase;

public final class UnoRuntime_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] {
            "test_generateOid", "test_queryInterface", "test_areSame",
            "test_currentContext" };
    }

    public void test_generateOid() {
        // Test if UnoRuntime generates an OID for a simple class:
        assure("Test1", UnoRuntime.generateOid(new Test1()) != null);

        // Test if UnoRuntime generates an OID for a class implementing
        // IQueryInterface and returning null from getOid:
        assure("Test2", UnoRuntime.generateOid(new Test2()) != null);

        // Test if a delegator object has the same OID as its creator:
        Test4 test4 = new Test4();
        Ifc ifc = (Ifc) UnoRuntime.queryInterface(Ifc.class, test4);
        assure(
            "Test4",
            UnoRuntime.generateOid(test4).equals(UnoRuntime.generateOid(ifc)));
    }

    public void test_queryInterface() {
        // Test if a query for an interface which is not supported returns null:
        assure(
            "Test1",
            (Ifc) UnoRuntime.queryInterface(Ifc.class, new Test1()) == null);

        // Test if a query for an interface which is supported through
        // IQueryInterface succeeds:
        assure(
            "Test2",
            (Ifc) UnoRuntime.queryInterface(Ifc.class, new Test2()) != null);

        // Test if a query for an interface which is directly supported (through
        // inheritance) succeeds:
        assure(
            "Test3",
            (Ifc) UnoRuntime.queryInterface(Ifc.class, new Test3()) != null);
    }

    public void test_areSame() {
        // Test if two independent instances are not the same:
        assure("Test1", !UnoRuntime.areSame(new Test1(), new Test2()));

        // Test if a delegator object is the same as its creator:
        Test2 test2 = new Test2();
        Ifc ifc = (Ifc) UnoRuntime.queryInterface(Ifc.class, test2);
        assure("Test2", UnoRuntime.areSame(ifc, test2));
    }

    public void test_currentContext() throws InterruptedException {
        TestThread t1 = new TestThread();
        TestThread t2 = new TestThread();
        t1.start();
        t2.start();
        t1.join();
        t2.join();
        Object v1 = t1.context.getValueByName("");
        Object v2 = t2.context.getValueByName("");
        assure("", t1.context != t2.context);
        assure("", v1 == t1);
        assure("", v2 == t2);
        assure("", v1 != v2);
    }

    private interface Ifc extends XInterface {}

    private static class Test1 {}

    private static class Test2 implements IQueryInterface {
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

    private static class Test4 implements IQueryInterface {
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
            assure("", UnoRuntime.getCurrentContext() == null);
            context = new TestCurrentContext();
            UnoRuntime.setCurrentContext(context);
            assure("", UnoRuntime.getCurrentContext() == context);
            assure("", context.getValueByName("") == this);
            UnoRuntime.setCurrentContext(null);
            assure("", UnoRuntime.getCurrentContext() == null);
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
