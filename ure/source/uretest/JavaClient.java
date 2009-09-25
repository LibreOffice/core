/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JavaClient.java,v $
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
        XBridgeFactory factory = UnoRuntime.queryInterface(
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
        Data d = UnoRuntime.queryInterface(
            XServer.class, bridge.getInstance(url.getRootOid())).getData();
        UnoRuntime.queryInterface(XComponent.class, bridge).dispose();
        if (!d.m1.equals("Hello") || d.m2 != 42) {
            throw new RuntimeException("Data object contains bad values");
        }
    }

    private JavaClient() {}
}
