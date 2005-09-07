/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DemoServer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:07:43 $
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

        XAcceptor xAcceptor = (XAcceptor)UnoRuntime.queryInterface(XAcceptor.class, serviceManager.createInstance("com.sun.star.connection.Acceptor"));

        System.err.println("waiting for connect...");
        XConnection xConnection = xAcceptor.accept(conDcp);

        XBridgeFactory xBridgeFactory = (XBridgeFactory)UnoRuntime.queryInterface(XBridgeFactory.class, serviceManager.createInstance("com.sun.star.bridge.BridgeFactory"));
        XBridge xBridge = xBridgeFactory.createBridge(conDcp + ";" + protDcp, protDcp, xConnection, new InstanceProvider());

    }
}
