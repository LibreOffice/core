/*************************************************************************
 *
 *  $RCSfile: java_remote_bridge_Test.java,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-09 10:14:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
import complexlib.ComplexTestCase;
import util.WaitUnreachable;

public final class java_remote_bridge_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
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
        assure("return null",
               bridgeB.getInstance(TestInstanceProvider.NAME_NULL) == null);

        try {
            bridgeB.getInstance(TestInstanceProvider.NAME_RUNTIME_EXCEPTION);
            failed("throw RuntimeException");
        } catch (com.sun.star.uno.RuntimeException e) {
            assure("throw RuntimeException",
                   e.getMessage().indexOf(
                       TestInstanceProvider.NAME_RUNTIME_EXCEPTION) != -1);
        }

        try {
            bridgeB.getInstance(
                TestInstanceProvider.NAME_NO_SUCH_ELEMENT_EXCEPTION);
            failed("throw NoSuchElementException");
        } catch (com.sun.star.uno.RuntimeException e) {
            assure("throw NoSuchElementException",
                   e.getMessage().indexOf(
                       TestInstanceProvider.NAME_NO_SUCH_ELEMENT_EXCEPTION)
                   != -1);
        }

        try {
            bridgeA.getInstance(TestInstanceProvider.NAME_ANYTHING);
            failed("no instance provider");
        } catch (com.sun.star.uno.RuntimeException e) {
            assure("no instance provider",
                   e.getMessage().indexOf("no instance provider set") != -1);
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
            proxyBTestInterface[i] = (TestInterface) UnoRuntime.queryInterface(
                TestInterface.class, proxyBXInterface[i]);
            proxyBTestInterface[i].function();

            // remap object once:
            TestInterface remapped = (TestInterface) UnoRuntime.queryInterface(
                TestInterface.class, proxyBXInterface[i]);
            remapped.function();

            // remap object twice:
            remapped = (TestInterface) UnoRuntime.queryInterface(
                TestInterface.class, proxyBXInterface[i]);
            remapped.function();
        }

        assure("calls of object method", TestProxy.getCount() == 3 * COUNT);

        // The following checks rely on the implementation detail that mapping
        // different facets of a UNO object (XInterface and TestInterface) leads
        // to different proxies:

        assure("bridge A life count", bridgeA.getLifeCount() == 2 * COUNT);
        assure("bridge B life count", bridgeB.getLifeCount() == 2 * COUNT);
        assure("proxy count", ProxyFactory.getDebugCount() == 2 * COUNT);

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
        // before the following assure is executed:
        Thread.sleep(1000);

        assure("proxy count", ProxyFactory.getDebugCount() == 0);

        System.out.println("waiting for pending messages to be done");
        while (bridgeB.getProtocol().getRequestsSendCount()
               > bridgeA.getProtocol().getRequestsReceivedCount()) {
            System.out.println(
                "pending: " + bridgeB.getProtocol().getRequestsSendCount()
                + ", " + bridgeA.getProtocol().getRequestsReceivedCount());
            Thread.sleep(100);
        }

        assure("Zero bridge A life count", bridgeA.getLifeCount() == 0);
        assure("Zero bridge B life count", bridgeB.getLifeCount() == 0);
        assure("Zero proxy count", ProxyFactory.getDebugCount() == 0);
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
        implements com.sun.star.lib.uno.Proxy, IQueryInterface, XInterface,
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
