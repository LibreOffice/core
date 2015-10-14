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
package com.sun.star.comp.bridge;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.container.XSet;

import com.sun.star.connection.Acceptor;
import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;

import com.sun.star.uno.UnoRuntime;

public class TestComponentMain
{

    private static class InstanceProvider implements XInstanceProvider {
        private XComponentContext ctx;

        public InstanceProvider( XComponentContext ctx )
        {
            this.ctx = ctx;
        }

        public Object getInstance( /*IN*/String sInstanceName )
            throws com.sun.star.container.NoSuchElementException, com.sun.star.uno.RuntimeException
        {
            Object o =null;
            try
            {
                o = ctx.getServiceManager().createInstanceWithContext(
                    "com.sun.star.comp.bridge.TestComponent$_TestObject" , ctx );
            }
            catch( com.sun.star.uno.Exception e )
            {
                System.out.println( "error during instantiation" + e );
            }
            return o;
        }
    }

    public static void main(String args[]) throws Exception, com.sun.star.uno.Exception {
        if(args.length != 2)    {
            System.err.println("usage : com.sun.star.comp.bridge.TestComponentMain uno:connection;protocol;objectName singleaccept");
            System.exit(-1);
        }

        String conDcp = null;
        String protDcp = null;

        String dcp = args[0];
        boolean singleaccept = args[1].equals("singleaccept");

        int index = dcp.indexOf(':');
        dcp = dcp.substring(index + 1).trim();

        index = dcp.indexOf(';');
        conDcp = dcp.substring(0, index).trim();
        dcp = dcp.substring(index + 1).trim();

        index = dcp.indexOf(';');
        protDcp = dcp.substring(0, index).trim();
        dcp = dcp.substring(index + 1).trim();

        XComponentContext ctx = com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );
        XMultiComponentFactory smgr = ctx.getServiceManager();
        XMultiServiceFactory oldsmgr =
            UnoRuntime.queryInterface( XMultiServiceFactory.class, smgr );

        // prepare servicemanager
        XSet set = UnoRuntime.queryInterface(XSet.class, smgr);
        Object o = com.sun.star.comp.bridge.TestComponent.__getServiceFactory(
            "com.sun.star.comp.bridge.TestComponent$_TestObject", oldsmgr,null );
        set.insert(o);

        XAcceptor xAcceptor = Acceptor.create(ctx);

        while( true )
        {
            System.err.println("waiting for connect...");

            XConnection xConnection = xAcceptor.accept(conDcp);

            XBridgeFactory xBridgeFactory = UnoRuntime.queryInterface(
                XBridgeFactory.class,
                smgr.createInstanceWithContext("com.sun.star.bridge.BridgeFactory",ctx));

            XBridge xBridge = xBridgeFactory.createBridge(
                "", protDcp, xConnection, new InstanceProvider(ctx));

            if (singleaccept) {
                Listener listener = new Listener();
                UnoRuntime.queryInterface(XComponent.class, xBridge).
                    addEventListener(listener);
                listener.await();
                break;
            }
        }

    }

    private static final class Listener implements XEventListener {
        public synchronized void disposing(EventObject source) {
            done = true;
            notifyAll();
        }

        public synchronized void await() {
            while (!done) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    throw new RuntimeException(e);
                }
            }
        }

        private boolean done = false;
    }
}
