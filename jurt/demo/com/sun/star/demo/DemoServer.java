/*************************************************************************
 *
 *  $RCSfile: DemoServer.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:27:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
