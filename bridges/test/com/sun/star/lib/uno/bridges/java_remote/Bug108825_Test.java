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

import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lib.TestBed;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;

/**
 * Test case for bug #108825#.
 *
 * <p>Bug #108825# "Java UNO Remote Bridge: Mapped-out Objects Not Held" shows
 * that local objects that are mapped out via a remote bridge, but not held
 * locally, might be garbage collected while there are still remote references
 * to them.  This test is not guaranteed to always work reliably, see comment in
 * the code.</p>
 */
public final class Bug108825_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        TestBed t = new TestBed();
        assure("test", t.execute(new Provider(t), true, Client.class, 0));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            XTest test = UnoRuntime.queryInterface(
                XTest.class, getBridge(context).getInstance("Test"));
            // Send the XObject that is held on the server side amidst two
            // dummies that are not held on the server side; then wait for the
            // dummies to be garbage collected, hoping that the XObject, if it
            // is erroneously not held on the client side, will be garbage
            // collected, too.  Obviously, this is not guaranteed to always work
            // (the VM might chose not to garbage collect the dummies, hanging
            // the test forever; or the VM might chose to garbage collect the
            // dummies but not the XObject, making the test pass erroneously).
            test.offer(new Dummy(), new XObject() { public void call() {} },
                       new Dummy());
            System.out.println("Client waiting for garbage collection...");
            for (;;) {
                synchronized (lock) {
                    if (finalizedCount == 2) {
                        break;
                    }
                }
                test.remoteGc();
                gc();
            }
            System.out.println("Client garbage collection done.");
            test.notification();
            return true;
        }

        private final class Dummy implements XDummy {
            protected void finalize() {
                synchronized (lock) {
                    ++finalizedCount;
                }
            }
        }

        private final Object lock = new Object();
        private int finalizedCount = 0;
    }

    // Make it as likely as possible that the VM reclaims all garbage:
    private static void gc() {
        System.gc();
        System.runFinalization();
        byte[] garbage = new byte[1024 * 1024];
    }

    private static final class Provider implements XInstanceProvider {
        public Provider(TestBed testBed) {
            this.testBed = testBed;
        }

        public Object getInstance(String instanceName) {
            return new XTest() {
                    public void offer(XDummy dummy1, XObject obj, XDummy dummy2)
                    {
                        this.obj = obj;
                    }

                    public void remoteGc() {
                        gc();
                    }

                    public void notification() {
                        obj.call();
                        testBed.serverDone(true);
                    }

                    private XObject obj;
                };
        }

        private final TestBed testBed;
    }

    public interface XDummy extends XInterface {
        TypeInfo[] UNOTYPEINFO = null;
    }

    public interface XObject extends XInterface {
        void call();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("call", 0, 0) };
    }

    public interface XTest extends XInterface {
        void offer(XDummy dummy1, XObject obj, XDummy dummy2);

        void remoteGc();

        void notification();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("offer", 0, 0),
                                   new MethodTypeInfo("remoteGc", 1, 0),
                                   new MethodTypeInfo("notification", 2, 0) };
    }
}
