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

package test.java.javaclient;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.connection.Connector;
import com.sun.star.lang.XComponent;
import com.sun.star.lib.uno.helper.UnoUrl;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import test.types.Data;
import test.types.XServer;

public final class JavaClient {
    public static void main(String[] arguments) throws Exception {
        XComponentContext context = Bootstrap.createInitialComponentContext(
            null);
        XBridgeFactory factory = UnoRuntime.queryInterface(
            XBridgeFactory.class,
            context.getServiceManager().createInstanceWithContext(
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
