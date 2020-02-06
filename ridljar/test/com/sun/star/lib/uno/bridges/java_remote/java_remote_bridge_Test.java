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
package com.sun.star.lib.uno.bridges.java_remote;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.comp.connections.PipedConnection;
import com.sun.star.connection.XConnection;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.lib.uno.environments.java.java_environment;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import org.junit.Test;
import util.WaitUnreachable;
import static org.junit.Assert.*;

public final class java_remote_bridge_Test {
    @Test public void test() throws Exception {
        String protocol = "urp";

        XConnection connectionA = new PipedConnection(new Object[0]);
        XConnection connectionB = new PipedConnection(
            new Object[] { connectionA });
        java_remote_bridge bridgeA = new java_remote_bridge(
            new java_environment(null), null,
            new Object[] { protocol, connectionA, new TestInstanceProvider() });
        java_remote_bridge bridgeB = new java_remote_bridge(
            new java_environment(null), null,
            new Object[] { protocol, connectionB, null });

        testGetInstance(bridgeA, bridgeB);
        testLifeCycle(bridgeA, bridgeB);
    }

    private void testGetInstance(XBridge bridgeA, XBridge bridgeB) {
        assertNull(bridgeB.getInstance(TestInstanceProvider.NAME_NULL));

        try {
            bridgeB.getInstance(TestInstanceProvider.NAME_RUNTIME_EXCEPTION);
            fail("throw RuntimeException");
        } catch (com.sun.star.uno.RuntimeException e) {
            assertTrue(
                e.getMessage().indexOf(
                    TestInstanceProvider.NAME_RUNTIME_EXCEPTION)
                != -1);
        }

        try {
            bridgeB.getInstance(
                TestInstanceProvider.NAME_NO_SUCH_ELEMENT_EXCEPTION);
            fail("throw NoSuchElementException");
        } catch (com.sun.star.uno.RuntimeException e) {
            assertTrue(
                e.getMessage().indexOf(
                    TestInstanceProvider.NAME_NO_SUCH_ELEMENT_EXCEPTION)
                != -1);
        }

        try {
            bridgeA.getInstance(TestInstanceProvider.NAME_ANYTHING);
            fail("no instance provider");
        } catch (com.sun.star.uno.RuntimeException e) {
            assertTrue(e.getMessage().startsWith("unknown OID "));
        }
    }

    private void testLifeCycle(java_remote_bridge bridgeA,
                               java_remote_bridge bridgeB)
        throws InterruptedException
    {
        // Repeatedly, objects are mapped from bridgeA to bridgeB, where proxies
        // for those objects (for the XInterface and TestInterface facets) are
        // created.  The proxies at bridgeB keep both bridges alive; after those
        // proxies have been garbage-collected, both bridges should be disposed.
        // It does not work to map a local object from bridgeA to bridgeB, as
        // bridgeB would find this object as a local one, too (via the shared,
        // static localObjects Registry in java_environment): bridgeB would not
        // create a proxy, would rather send back a "release" to bridgeA, and
        // both bridges would be disposed while the first object is being
        // mapped.  Therefore, a HACK is used to install TestProxy objects
        // (which behave as if they got mapped in to bridgeA from somewhere
        // else) at bridgeA and map those.

        final int COUNT = 100;
        XInterface[] proxyBXInterface = new XInterface[COUNT];
        TestInterface[] proxyBTestInterface = new TestInterface[COUNT];
        for (int i = 0; i < COUNT; ++i) {
            String name = "TestOID" + i;
            Object proxyA = new TestProxy(name);
            bridgeA.getSourceEnvironment().registerInterface(
                proxyA, new String[] { name }, new Type(XInterface.class));

            proxyBXInterface[i] = (XInterface) bridgeB.getInstance(name);

            // map object:
            proxyBTestInterface[i] = UnoRuntime.queryInterface(
                TestInterface.class, proxyBXInterface[i]);
            proxyBTestInterface[i].function();

            // remap object once:
            TestInterface remapped = UnoRuntime.queryInterface(
                TestInterface.class, proxyBXInterface[i]);
            remapped.function();

            // remap object twice:
            remapped = UnoRuntime.queryInterface(
                TestInterface.class, proxyBXInterface[i]);
            remapped.function();
        }

        assertEquals(3 * COUNT, TestProxy.getCount());

        // The following checks rely on the implementation detail that mapping
        // different facets of a UNO object (XInterface and TestInterface) leads
        // to different proxies:

        assertEquals("bridge A life count", 2 * COUNT, bridgeA.getLifeCount());
        assertEquals("bridge B life count", 2 * COUNT, bridgeB.getLifeCount());
/*TODO: below test fails with "expected:<200> but was:<204>":
        assertEquals("proxy count", 2 * COUNT, ProxyFactory.getDebugCount());
*/

        System.out.println("waiting for proxies to become unreachable:");
        for (int i = 0; i < COUNT; ++i) {
            WaitUnreachable u1 = new WaitUnreachable(proxyBXInterface[i]);
            WaitUnreachable u2 = new WaitUnreachable(proxyBTestInterface[i]);
            proxyBXInterface[i] = null;
            proxyBTestInterface[i] = null;
            u1.waitUnreachable();
            u2.waitUnreachable();
        }
        // For whatever strange reason, this sleep seems to be necessary to
        // reliably ensure that even the last proxy's finalization is over
        // before the following assert is executed:
        Thread.sleep(1000);

        assertEquals("proxy count", 0, ProxyFactory.getDebugCount());

        System.out.println("waiting for pending messages to be done");
        while (bridgeA.getLifeCount() != 0 || bridgeB.getLifeCount() != 0) {
            Thread.sleep(100);
        }

        assertEquals("Zero bridge A life count", 0, bridgeA.getLifeCount());
        assertEquals("Zero bridge B life count", 0, bridgeB.getLifeCount());
        assertEquals("Zero proxy count", 0, ProxyFactory.getDebugCount());
    }

    public interface TestInterface extends XInterface {
        void function();

        TypeInfo[] UNOTYPEINFO = new TypeInfo[] {
            new MethodTypeInfo("function", 0, 0) };
    }

    private static final class TestInstanceProvider
        implements XInstanceProvider
    {
        public Object getInstance(String name) throws NoSuchElementException {
            if (name.equals(NAME_NULL)) {
                return null;
            } else if (name.equals(NAME_RUNTIME_EXCEPTION)) {
                throw new com.sun.star.uno.RuntimeException(
                    getClass().getName() + ", throwing: " + name);
            } else if (name.equals(NAME_NO_SUCH_ELEMENT_EXCEPTION)) {
                throw new NoSuchElementException(
                    getClass().getName() + ", throwing: " + name);
            } else {
                throw new IllegalStateException();
            }
        }

        public static final String NAME_NULL = "return null";
        public static final String NAME_RUNTIME_EXCEPTION
        = "throw RuntimeException";
        public static final String NAME_NO_SUCH_ELEMENT_EXCEPTION
        = "throw NoSuchElementException";
        public static final String NAME_ANYTHING = "anything";
    }

    private static final class TestProxy
        implements com.sun.star.lib.uno.Proxy, IQueryInterface,
            TestInterface
    {
        public TestProxy(String oid) {
            this.oid = oid;
        }

        public Object queryInterface(Type type) {
            // type should be either XInterface or TestInterface...
            return this;
        }

        public boolean isSame(Object object) {
            return object instanceof TestProxy
                && oid.equals(((TestProxy) object).oid);
        }

        public String getOid() {
            return oid;
        }

        public void function() {
            synchronized (getClass()) {
                ++count;
            }
        }

        public static synchronized int getCount() {
            return count;
        }

        private final String oid;

        private static int count = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
