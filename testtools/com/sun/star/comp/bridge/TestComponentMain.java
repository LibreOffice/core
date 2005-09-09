/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestComponentMain.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:21:10 $
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
package com.sun.star.comp.bridge;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.container.XSet;
import com.sun.star.loader.XImplementationLoader;

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
        if(args.length != 1)    {
            System.err.println("usage : com.sun.star.comp.bridge.TestComponentMain uno:connection;protocol;objectName");
            System.exit(-1);
        }

        String conDcp = null;
        String protDcp = null;
        String rootOid = null;

        String dcp = args[0];

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

        System.err.println("waiting for connect...");

        while( true )
        {
            XConnection xConnection = xAcceptor.accept(conDcp);

            XBridgeFactory xBridgeFactory = (XBridgeFactory)UnoRuntime.queryInterface(
                XBridgeFactory.class,
                smgr.createInstanceWithContext("com.sun.star.bridge.BridgeFactory",ctx));

            XBridge xBridge = xBridgeFactory.createBridge(
                "", protDcp, xConnection, new InstanceProvider(ctx));
        }

    }
}
