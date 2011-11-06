/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package com.sun.star.comp.bridgefactory;

import com.sun.star.bridge.BridgeExistsException;
import com.sun.star.bridge.XBridge;
import com.sun.star.comp.connections.PipedConnection;
import com.sun.star.connection.XConnection;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;

public final class BridgeFactory_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        PipedConnection rightSide = new PipedConnection(new Object[0]);
        PipedConnection leftSide = new PipedConnection(new Object[]{rightSide});

        BridgeFactory bridgeFactory = new BridgeFactory(); // create the needed bridgeFactory

        // create a bridge
        XBridge xBridge = bridgeFactory.createBridge("testbridge", "urp", (XConnection)leftSide, null);

        // test that we get the same bridge
        assure("", UnoRuntime.areSame(xBridge,
                                      bridgeFactory.getBridge("testbridge")));

        // test that we can not create another bridge with same name
        try {
            XBridge dummy = bridgeFactory.createBridge("testbridge", "urp", (XConnection)leftSide, null);

            failed("");
        }
        catch(BridgeExistsException bridgeExistsException) {
        }


        // test getExistingBridges
        XBridge xBridges[] = bridgeFactory.getExistingBridges();
        assure("", UnoRuntime.areSame(xBridge, xBridges[0]));

        // dispose the bridge
        XComponent xComponent = UnoRuntime.queryInterface(XComponent.class, xBridge);
        xComponent.dispose();


        // test that the bridge has been removed
        assure("", bridgeFactory.getBridge("testbridge") == null);



        rightSide = new PipedConnection(new Object[0]);
        leftSide = new PipedConnection(new Object[]{rightSide});


        // test that we really get a new bridge
        XBridge xBridge_new = bridgeFactory.createBridge("testbridge", "urp", (XConnection)leftSide, null);
        assure("", !UnoRuntime.areSame(xBridge, xBridge_new));

        for(int i = 0; i <10000; ++ i) {
            Object x[] = new Object[100];
        }

        // test getExistingBridges
        xBridges = bridgeFactory.getExistingBridges();
        assure("",
               xBridges.length == 1
               && UnoRuntime.areSame(xBridge_new, xBridges[0]));

        // dispose the new bridge
        XComponent xComponent_new = UnoRuntime.queryInterface(XComponent.class, xBridge_new);
        xComponent_new.dispose();
    }
}
