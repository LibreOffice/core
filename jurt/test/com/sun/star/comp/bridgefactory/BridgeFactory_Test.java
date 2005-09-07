/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BridgeFactory_Test.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:09:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
        XComponent xComponent = (XComponent)UnoRuntime.queryInterface(XComponent.class, xBridge);
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
        XComponent xComponent_new = (XComponent)UnoRuntime.queryInterface(XComponent.class, xBridge_new);
        xComponent_new.dispose();
    }
}
