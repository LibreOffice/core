/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Bug98508_Test.java,v $
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
import com.sun.star.lang.DisposedException;
import com.sun.star.lib.TestBed;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import complexlib.ComplexTestCase;

/**
 * Test case for bug #98508#.
 *
 * <p>Bug #98508# "JAVA UNO bridge is not disposed when Exception occures during
 * sendReply()" states that the server returning <code>null</code> instead of a
 * valid <code>String</code> from <code>XServiceName.getServiceName</code>
 * causes an exception when sending the reply, but this exception did not cause
 * the bridge to be disposed, it rather caused both client and server to
 * hang.</p>
 *
 * <p>Since null instead of a <code>String</code> no longer causes an exception
 * in the bridge, this test has been redesigned to send a value of a wrong
 * instantiated polymorphic struct type instead.</p>
 *
 * <p>This test has to detect whether the spawned client process indeed hangs,
 * which can not be done reliably.  As an approximation, it waits for 10 sec and
 * considers the process hanging if it has not completed by then.</p>
 */
public final class Bug98508_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        TestBed t = new TestBed();
        assure("test", t.execute(new Provider(t), true, Client.class, 10000));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            Test98508Interface ifc
                = UnoRuntime.queryInterface(
                    Test98508Interface.class,
                    getBridge(context).getInstance(""));
            try {
                ifc.get();
            } catch (DisposedException e) {
                return true;
            }
            return false;
        }
    }

    private static final class Provider implements XInstanceProvider {
        public Provider(TestBed testBed) {
            this.testBed = testBed;
        }

        public Object getInstance(String instanceName) {
            return new Test98508Interface() {
                    public Test98508Struct get() {
                        testBed.serverDone(true);
                        return new Test98508Struct(Boolean.FALSE);
                    }
                };
        }

        private final TestBed testBed;
    }
}
