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

import com.sun.star.comp.connections.PipedConnection;
import complexlib.ComplexTestCase;
import util.WaitUnreachable;

public final class UnoRuntime_EnvironmentTest extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test_getEnvironment", "test_getBridge" };
    }

    public void test_getEnvironment() throws java.lang.Exception {
        Object o1 = new Object();
        Object o2 = new Object();

        // get two environments with different contexts
        WaitUnreachable java_environment1 = new WaitUnreachable(
            UnoRuntime.getEnvironment("java", o1));
        WaitUnreachable java_environment2 = new WaitUnreachable(
            UnoRuntime.getEnvironment("java", o2));

        // ensure that the environments are different
        assure("", java_environment1.get() != java_environment2.get());

        // test if we get the same environment when we reget it
        assure("",
               UnoRuntime.areSame(java_environment1.get(),
                                  UnoRuntime.getEnvironment("java", o1)));
        assure("",
               UnoRuntime.areSame(java_environment2.get(),
                                  UnoRuntime.getEnvironment("java", o2)));

        // drop the environments and wait until they are gc
        java_environment1.waitUnreachable();
        java_environment2.waitUnreachable();
    }

    public void test_getBridge() throws java.lang.Exception {
        PipedConnection conn = new PipedConnection(new Object[0]);
        new PipedConnection(new Object[] { conn });

        // get a bridge
        IBridge iBridge = UnoRuntime.getBridgeByName(
            "java", null, "remote", "testname",
            new Object[] { "urp", conn, null });

        // reget the bridge, it must be the same as above
        IBridge iBridge_tmp = UnoRuntime.getBridgeByName(
            "java", null, "remote", "testname",
            new Object[] { "urp", conn, null });
        assure("", UnoRuntime.areSame(iBridge_tmp, iBridge));

        // dispose the bridge, this removes the entry from the runtime
        iBridge.dispose();

        conn = new PipedConnection(new Object[0]);
        new PipedConnection(new Object[] { conn });

        // reget the bridge, it must be a different one
        iBridge_tmp = UnoRuntime.getBridgeByName(
            "java", null, "remote", "testname",
            new Object[]{ "urp", conn, null });
        assure("", !UnoRuntime.areSame(iBridge_tmp, iBridge));
    }
}
