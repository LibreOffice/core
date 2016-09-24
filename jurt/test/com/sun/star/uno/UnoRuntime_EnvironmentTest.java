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

package com.sun.star.uno;

import com.sun.star.comp.connections.PipedConnection;
import org.junit.Test;
import util.WaitUnreachable;
import static org.junit.Assert.*;

public final class UnoRuntime_EnvironmentTest {
    @Test public void test_getEnvironment() throws java.lang.Exception {
        Object o1 = new Object();
        Object o2 = new Object();

        // get two environments with different contexts
        WaitUnreachable java_environment1 = new WaitUnreachable(
            UnoRuntime.getEnvironment("java", o1));
        WaitUnreachable java_environment2 = new WaitUnreachable(
            UnoRuntime.getEnvironment("java", o2));

        // ensure that the environments are different
        assertNotSame(java_environment1.get(), java_environment2.get());

        // test if we get the same environment when we reget it
        assertTrue(
            UnoRuntime.areSame(
                java_environment1.get(),
                UnoRuntime.getEnvironment("java", o1)));
        assertTrue(
            UnoRuntime.areSame(
                java_environment2.get(),
                UnoRuntime.getEnvironment("java", o2)));

        // drop the environments and wait until they are gc
        java_environment1.waitUnreachable();
        java_environment2.waitUnreachable();
    }

    @Test public void test_getBridge() throws java.lang.Exception {
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
        assertTrue(UnoRuntime.areSame(iBridge_tmp, iBridge));

        // dispose the bridge, this removes the entry from the runtime
        iBridge.dispose();

        conn = new PipedConnection(new Object[0]);
        new PipedConnection(new Object[] { conn });

        // reget the bridge, it must be a different one
        iBridge_tmp = UnoRuntime.getBridgeByName(
            "java", null, "remote", "testname",
            new Object[]{ "urp", conn, null });
        assertFalse(UnoRuntime.areSame(iBridge_tmp, iBridge));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
