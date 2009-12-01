/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestComponentMain.java,v $
 * $Revision: 1.4.22.1 $
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

    static class InstanceProvider implements XInstanceProvider {
        XComponentContext ctx;

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

    static public void main(String args[]) throws Exception, com.sun.star.uno.Exception {
        if(args.length != 2)    {
            System.err.println("usage : com.sun.star.comp.bridge.TestComponentMain uno:connection;protocol;objectName singleaccept");
            System.exit(-1);
        }

        String conDcp = null;
        String protDcp = null;
        String rootOid = null;

        String dcp = args[0];
        boolean singleaccept = args[1].equals("singleaccept");

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

        XComponentContext ctx = com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );
        XMultiComponentFactory smgr = ctx.getServiceManager();
        XMultiServiceFactory oldsmgr = (XMultiServiceFactory)
            UnoRuntime.queryInterface( XMultiServiceFactory.class, smgr );

        // prepare servicemanager
        XSet set = (XSet) UnoRuntime.queryInterface(XSet.class, smgr);
        Object o = com.sun.star.comp.bridge.TestComponent.__getServiceFactory(
            "com.sun.star.comp.bridge.TestComponent$_TestObject", oldsmgr,null );
        set.insert(o);

        XAcceptor xAcceptor = Acceptor.create(ctx);

        while( true )
        {
            System.err.println("waiting for connect...");

            XConnection xConnection = xAcceptor.accept(conDcp);

            XBridgeFactory xBridgeFactory = (XBridgeFactory)UnoRuntime.queryInterface(
                XBridgeFactory.class,
                smgr.createInstanceWithContext("com.sun.star.bridge.BridgeFactory",ctx));

            XBridge xBridge = xBridgeFactory.createBridge(
                "", protDcp, xConnection, new InstanceProvider(ctx));

            if (singleaccept) {
                Listener listener = new Listener();
                ((XComponent) UnoRuntime.queryInterface(
                    XComponent.class, xBridge)).addEventListener(listener);
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
