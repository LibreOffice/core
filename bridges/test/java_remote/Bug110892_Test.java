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

package test.java_remote;

import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import util.WaitUnreachable;
import test.lib.TestBed;

/**
 * Test case for bug #110892#.
 *
 * <p>Bug #110892# "Java URP bridge holds objects indefinitely" applies to cases
 * where an object is sent from server to client, then recursively back from
 * client to server.  In such a case, the client should not increment its
 * internal reference count for the object, as the server will never send back a
 * corresponding release message.</p>
 *
 * <p>This test has to detect whether the spawned client process fails to
 * garbage-collect an object, which can not be done reliably.  As an
 * approximation, it waits for 10 sec and considers the process failing if it
 * has not garbage-collected the object by then.</p>
 */
public final class Bug110892_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        assure("test",
               new TestBed().execute(new Provider(), false, Client.class,
                                     10000));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            XTest test = UnoRuntime.queryInterface(
                XTest.class, getBridge(context).getInstance("Test"));
            test.start(new ClientObject());
            synchronized (lock) {
                unreachable.waitUnreachable();
            }
            return true;
        }

        private final class ClientObject implements XClientObject {
            public void call(XServerObject server, XInterface object) {
                synchronized (lock) {
                    unreachable = new WaitUnreachable(object);
                }
                server.call(object);
            }
        }

        private final Object lock = new Object();
        private WaitUnreachable unreachable = null;
    }

    private static final class Provider implements XInstanceProvider {
        public Object getInstance(String instanceName) {
            return new XTest() {
                    public void start(XClientObject client) {
                        client.call(
                            new XServerObject() {
                                public void call(XInterface object) {}
                            },
                            new XInterface() {});
                    }
                };
        }
    }

    public interface XClientObject extends XInterface {
        void call(XServerObject server, XInterface object);

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("call", 0, 0) };
    }

    public interface XServerObject extends XInterface {
        void call(XInterface object);

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("call", 0, 0) };
    }

    public interface XTest extends XInterface {
        void start(XClientObject client);

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("start", 0, 0) };
    }
}
