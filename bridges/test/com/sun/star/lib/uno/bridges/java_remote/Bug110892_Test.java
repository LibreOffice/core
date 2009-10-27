/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Bug110892_Test.java,v $
 * $Revision: 1.6 $
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

package com.sun.star.lib.uno.bridges.javaremote;

import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lib.TestBed;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import util.WaitUnreachable;

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
