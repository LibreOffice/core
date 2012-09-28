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

package com.sun.star.comp.bridgefactory;

import com.sun.star.bridge.BridgeExistsException;
import com.sun.star.bridge.XBridge;
import com.sun.star.comp.connections.PipedConnection;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import org.junit.Test;
import static org.junit.Assert.*;

public final class BridgeFactory_Test {
    @Test public void test() throws Exception {
        PipedConnection rightSide = new PipedConnection(new Object[0]);
        PipedConnection leftSide = new PipedConnection(new Object[]{rightSide});

        BridgeFactory bridgeFactory = new BridgeFactory(); // create the needed bridgeFactory

        // create a bridge
        XBridge xBridge = bridgeFactory.createBridge("testbridge", "urp", leftSide, null);

        // test that we get the same bridge
        assertTrue(
            UnoRuntime.areSame(
                xBridge, bridgeFactory.getBridge("testbridge")));

        // test that we can not create another bridge with same name
        try {
            XBridge dummy = bridgeFactory.createBridge("testbridge", "urp", leftSide, null);

            fail();
        }
        catch(BridgeExistsException bridgeExistsException) {
        }


        // test getExistingBridges
        XBridge xBridges[] = bridgeFactory.getExistingBridges();
        assertTrue(UnoRuntime.areSame(xBridge, xBridges[0]));

        // dispose the bridge
        XComponent xComponent = UnoRuntime.queryInterface(XComponent.class, xBridge);
        xComponent.dispose();


        // test that the bridge has been removed
        assertTrue(bridgeFactory.getBridge("testbridge") == null);



        rightSide = new PipedConnection(new Object[0]);
        leftSide = new PipedConnection(new Object[]{rightSide});


        // test that we really get a new bridge
        XBridge xBridge_new = bridgeFactory.createBridge("testbridge", "urp", leftSide, null);
        assertFalse(UnoRuntime.areSame(xBridge, xBridge_new));

        for(int i = 0; i <10000; ++ i) {
            Object x[] = new Object[100];
        }

        // test getExistingBridges
        xBridges = bridgeFactory.getExistingBridges();
        assertEquals(1, xBridges.length);
        assertTrue(UnoRuntime.areSame(xBridge_new, xBridges[0]));

        // dispose the new bridge
        XComponent xComponent_new = UnoRuntime.queryInterface(XComponent.class, xBridge_new);
        xComponent_new.dispose();
    }
}
