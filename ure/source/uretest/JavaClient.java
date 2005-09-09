/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JavaClient.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:12:22 $
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

package test.java.javaclient;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.connection.Connector;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lib.uno.helper.UnoUrl;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import test.types.Data;
import test.types.XServer;

public final class JavaClient {
    public static void main(String[] arguments) throws Exception {
        XComponentContext context = Bootstrap.createInitialComponentContext(
            null);
        XMultiComponentFactory manager = context.getServiceManager();
        if (manager == null) {
            throw new NullPointerException("no service manager");
        }
        XBridgeFactory factory = (XBridgeFactory) UnoRuntime.queryInterface(
            XBridgeFactory.class,
            manager.createInstanceWithContext(
                "com.sun.star.bridge.BridgeFactory", context));
        if (factory == null) {
            throw new NullPointerException("no bridge factory");
        }
        UnoUrl url = UnoUrl.parseUnoUrl(arguments[0]);
        XBridge bridge = factory.createBridge(
            "", url.getProtocolAndParametersAsString(),
            Connector.create(context).connect(
                url.getConnectionAndParametersAsString()),
            null);
        Data d =
            ((XServer) UnoRuntime.queryInterface(
                XServer.class, bridge.getInstance(url.getRootOid()))).getData();
        ((XComponent) UnoRuntime.queryInterface(XComponent.class, bridge)).
            dispose();
        if (!d.m1.equals("Hello") || d.m2 != 42) {
            throw new RuntimeException("Data object contains bad values");
        }
    }

    private JavaClient() {}
}
