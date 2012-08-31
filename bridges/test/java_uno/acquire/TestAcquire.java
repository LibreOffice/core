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

package test.java_uno.acquire;

import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.connection.Acceptor;
import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.lib.uno.helper.UnoUrl;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import util.WaitUnreachable;

public final class TestAcquire {
    public static void main(String[] arguments) throws Exception {
        // - arguments[0] must be "client" or "server"
        // - arguments[1] must be the UNO URL to connect to (client) or accept
        //   on (server)
        XComponentContext context
            = Bootstrap.createInitialComponentContext(null);
        if (arguments[0].equals("client")) {
            execClient(context, arguments[1]);
        } else {
            execServer(context, arguments[1]);
        }
    }

    private static void assertNotNull(Object obj) {
        if (obj == null) {
            throw new RuntimeException("assertNotNull failed");
        }
    }

    private static void receive(Object obj) {
        assertNotNull(obj);
        WaitUnreachable.ensureFinalization(obj);
    }

    private static void execClient(XComponentContext context, String url)
        throws Exception
    {
        XTest test = UnoRuntime.queryInterface(
            XTest.class, UnoUrlResolver.create(context).resolve(url));

        WaitUnreachable u;

        u = new WaitUnreachable(new XInterface() {});
        test.setInterfaceToInterface((XInterface) u.get());
        receive(test.getInterfaceFromInterface());
        test.clearInterface();
        u.waitUnreachable();
        u = new WaitUnreachable(new XBase() {});
        test.setInterfaceToInterface((XBase) u.get());
        receive(test.getInterfaceFromInterface());
        test.clearInterface();
        u.waitUnreachable();
        u = new WaitUnreachable(new XDerived() {});
        test.setInterfaceToInterface((XDerived) u.get());
        receive(test.getInterfaceFromInterface());
        test.clearInterface();
        u.waitUnreachable();

        u = new WaitUnreachable(new XBase() {});
        test.setBaseToInterface((XBase) u.get());
        receive(test.getInterfaceFromInterface());
        test.clearInterface();
        u.waitUnreachable();
        u = new WaitUnreachable(new XDerived() {});
        test.setBaseToInterface((XDerived) u.get());
        receive(test.getInterfaceFromInterface());
        test.clearInterface();
        u.waitUnreachable();

        u = new WaitUnreachable(new XDerived() {});
        test.setDerivedToInterface((XDerived) u.get());
        receive(test.getInterfaceFromInterface());
        test.clearInterface();
        u.waitUnreachable();

        u = new WaitUnreachable(new XBase() {});
        test.setBaseToBase((XBase) u.get());
        receive(test.getInterfaceFromBase());
        receive(test.getBaseFromBase());
        test.clearBase();
        u.waitUnreachable();
        u = new WaitUnreachable(new XDerived() {});
        test.setBaseToBase((XDerived) u.get());
        receive(test.getInterfaceFromBase());
        receive(test.getBaseFromBase());
        test.clearBase();
        u.waitUnreachable();

        u = new WaitUnreachable(new XDerived() {});
        test.setDerivedToBase((XDerived) u.get());
        receive(test.getInterfaceFromBase());
        receive(test.getBaseFromBase());
        test.clearBase();
        u.waitUnreachable();

        u = new WaitUnreachable(new XDerived() {});
        test.setDerivedToDerived((XDerived) u.get());
        receive(test.getInterfaceFromDerived());
        receive(test.getBaseFromDerived());
        receive(test.getDerivedFromDerived());
        test.clearDerived();
        u.waitUnreachable();

        u = new WaitUnreachable(new XInterface() {});
        receive(test.roundTripInterfaceToInterface((XInterface) u.get()));
        u.waitUnreachable();
        u = new WaitUnreachable(new XBase() {});
        receive(test.roundTripInterfaceToInterface((XBase) u.get()));
        u.waitUnreachable();
        u = new WaitUnreachable(new XDerived() {});
        receive(test.roundTripInterfaceToInterface((XDerived) u.get()));
        u.waitUnreachable();

        u = new WaitUnreachable(new XBase() {});
        receive(test.roundTripBaseToInterface((XBase) u.get()));
        u.waitUnreachable();
        u = new WaitUnreachable(new XDerived() {});
        receive(test.roundTripBaseToInterface((XDerived) u.get()));
        u.waitUnreachable();

        u = new WaitUnreachable(new XDerived() {});
        receive(test.roundTripDerivedToInterface((XDerived) u.get()));
        u.waitUnreachable();

        u = new WaitUnreachable(new XBase() {});
        receive(test.roundTripBaseToBase((XBase) u.get()));
        u.waitUnreachable();
        u = new WaitUnreachable(new XDerived() {});
        receive(test.roundTripBaseToBase((XDerived) u.get()));
        u.waitUnreachable();

        u = new WaitUnreachable(new XDerived() {});
        receive(test.roundTripDerivedToBase((XDerived) u.get()));
        u.waitUnreachable();

        u = new WaitUnreachable(new XDerived() {});
        receive(test.roundTripDerivedToDerived((XDerived) u.get()));
        u.waitUnreachable();

        u = new WaitUnreachable(test);
        test = null;
        u.waitUnreachable();
        System.out.println(
            "Client and server both cleanly terminate now: Success");
    }

    private static void execServer(XComponentContext context, String url)
        throws Exception
    {
        XAcceptor acceptor = Acceptor.create(context);
        XBridgeFactory factory = UnoRuntime.queryInterface(
            XBridgeFactory.class,
            context.getServiceManager().createInstanceWithContext(
                "com.sun.star.bridge.BridgeFactory", context));
        UnoUrl unoUrl = UnoUrl.parseUnoUrl(url);
        System.out.println("Server: Accepting...");
        XConnection connection = acceptor.accept(
            unoUrl.getConnectionAndParametersAsString());
        System.out.println("Server: ...connected...");
        factory.createBridge(
            "", unoUrl.getProtocolAndParametersAsString(), connection,
            new Provider());
        System.out.println("Server: ...bridged.");
    }

    private static final class Provider implements XInstanceProvider {
        public Object getInstance(String instanceName) {
            return new XTest() {
                    public void setInterfaceToInterface(Object obj) {
                        iface = obj;
                    }

                    public void setBaseToInterface(XBase obj) {
                        iface = obj;
                    }

                    public void setDerivedToInterface(XDerived obj) {
                        iface = obj;
                    }

                    public Object getInterfaceFromInterface() {
                        return iface;
                    }

                    public void clearInterface() {
                        WaitUnreachable u = new WaitUnreachable(iface);
                        iface = null;
                        u.waitUnreachable();
                    }

                    public void setBaseToBase(XBase obj) {
                        base = obj;
                    }

                    public void setDerivedToBase(XDerived obj) {
                        base = obj;
                    }

                    public Object getInterfaceFromBase() {
                        return base;
                    }

                    public XBase getBaseFromBase() {
                        return base;
                    }

                    public void clearBase() {
                        WaitUnreachable u = new WaitUnreachable(base);
                        base = null;
                        u.waitUnreachable();
                    }

                    public void setDerivedToDerived(XDerived obj) {
                        derived = obj;
                    }

                    public Object getInterfaceFromDerived() {
                        return derived;
                    }

                    public XBase getBaseFromDerived() {
                        return derived;
                    }

                    public XDerived getDerivedFromDerived() {
                        return derived;
                    }

                    public void clearDerived() {
                        WaitUnreachable u = new WaitUnreachable(derived);
                        derived = null;
                        u.waitUnreachable();
                    }

                    public Object roundTripInterfaceToInterface(Object obj) {
                        WaitUnreachable.ensureFinalization(obj);
                        return obj;
                    }

                    public Object roundTripBaseToInterface(XBase obj) {
                        WaitUnreachable.ensureFinalization(obj);
                        return obj;
                    }

                    public Object roundTripDerivedToInterface(XDerived obj) {
                        WaitUnreachable.ensureFinalization(obj);
                        return obj;
                    }

                    public XBase roundTripBaseToBase(XBase obj) {
                        WaitUnreachable.ensureFinalization(obj);
                        return obj;
                    }

                    public XBase roundTripDerivedToBase(XDerived obj) {
                        WaitUnreachable.ensureFinalization(obj);
                        return obj;
                    }

                    public XDerived roundTripDerivedToDerived(XDerived obj) {
                        WaitUnreachable.ensureFinalization(obj);
                        return obj;
                    }

                    private Object iface;
                    private XBase base;
                    private XDerived derived;
                };
        }
    }
}
