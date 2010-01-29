/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DemoServer.java,v $
 * $Revision: 1.3 $
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
package com.sun.star.demo;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.comp.servicemanager.ServiceManager;

import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;

import com.sun.star.uno.UnoRuntime;

public class DemoServer {
    static String neededServices[] = new String[] {
        "com.sun.star.comp.servicemanager.ServiceManager",
        "com.sun.star.comp.loader.JavaLoader",
        "com.sun.star.comp.connections.Acceptor",
        "com.sun.star.comp.bridgefactory.BridgeFactory"
    };

    static class InstanceProvider implements XInstanceProvider {
        public Object getInstance( /*IN*/String sInstanceName ) throws com.sun.star.container.NoSuchElementException, com.sun.star.uno.RuntimeException {
            System.err.println("##### " + getClass().getName() + ".getInstance:" + sInstanceName);

            return null;
        }
    }

    static public void main(String args[]) throws Exception {
        if(args.length != 1)    {
            System.err.println("usage : SCalc uno:connection;protocol;objectName");
            System.exit(-1);
        }

        String conDcp = null;
        String protDcp = null;
        String rootOid = null;

        String dcp = args[0];

        if(dcp.indexOf(';') == -1) {// use old style
            conDcp = dcp;
            protDcp = "iiop";
            rootOid = "classic_uno";
        }
        else { // new style
            int index = dcp.indexOf(':');
            String url = dcp.substring(0, index).trim();
            dcp = dcp.substring(index + 1).trim();

            index = dcp.indexOf(';');
            conDcp = dcp.substring(0, index).trim();
            dcp = dcp.substring(index + 1).trim();

            index = dcp.indexOf(';');
            protDcp = dcp.substring(0, index).trim();
            dcp = dcp.substring(index + 1).trim();

            rootOid = dcp.trim().trim();
        }

        ServiceManager serviceManager = new ServiceManager();
        serviceManager.addFactories(neededServices);

        XAcceptor xAcceptor = UnoRuntime.queryInterface(XAcceptor.class, serviceManager.createInstance("com.sun.star.connection.Acceptor"));

        System.err.println("waiting for connect...");
        XConnection xConnection = xAcceptor.accept(conDcp);

        XBridgeFactory xBridgeFactory = UnoRuntime.queryInterface(XBridgeFactory.class, serviceManager.createInstance("com.sun.star.bridge.BridgeFactory"));
        XBridge xBridge = xBridgeFactory.createBridge(conDcp + ";" + protDcp, protDcp, xConnection, new InstanceProvider());

    }
}
