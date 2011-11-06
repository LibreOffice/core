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
