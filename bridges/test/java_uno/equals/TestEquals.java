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

package test.java_uno.equals;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.connection.Acceptor;
import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.loader.XImplementationLoader;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import java.io.File;
import java.net.MalformedURLException;
import java.util.HashMap;
import java.util.Hashtable;
import test.lib.TestBed;

// In this test scenario, the Java server (see implementation of method
// notifyAccepting) has a remote bridge to the Java client and a local JNI
// bridge to a C++ com.sun.star.test.bridges.testequals.impl service.  The C++
// service and the Java client are also connected via a remote bridge.
//
// The Java server gets two objects (INSTANCE1, INSTANCE2), once directly from
// the Java client via the remote bridge (proxies test1A, test2A), and once
// through the C++ service via the JNI bridge (proxies test1B, test2B).
// Exhaustive tests on the proxies' equals and hashCode methods are done.

public final class TestEquals {
    // args[0] must be a file system path to a types.rdb,
    // args[1] must be a file system path to a services.rdb
    public static void main(String[] args) throws Exception {
        TestBed t = new TestBed();
        boolean success = t.execute(
            new Provider(t, toFileUrl(args[0]), toFileUrl(args[1])), true,
            Client.class, 0);
        System.out.println("success? " + success);
        System.exit(success ? 0 : 1);
    }

    private static String toFileUrl(String path) throws MalformedURLException {
        String url = new File(path).toURI().toURL().toString();
        String prefix = "file:/";
        if (url.startsWith(prefix)
            && (url.length() == prefix.length()
                || url.charAt(prefix.length()) != '/')) {
            url = url.substring(0, prefix.length()) + "//"
                + url.substring(prefix.length());
        }
        return url;
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            XTestFrame f = UnoRuntime.queryInterface(
                XTestFrame.class, getBridge(context).getInstance("TestFrame"));
            XAcceptor acceptor = Acceptor.create(context);
            XBridgeFactory factory = UnoRuntime.queryInterface(
                XBridgeFactory.class,
                context.getServiceManager().createInstanceWithContext(
                    "com.sun.star.bridge.BridgeFactory", context));
            System.out.println("Client, 2nd connection: Accepting...");
            XInstanceProvider prov = new Provider();
            f.notifyAccepting(new Done(), prov.getInstance(INSTANCE1),
                              prov.getInstance(INSTANCE2));
            XConnection connection = acceptor.accept(CONNECTION_DESCRIPTION);
            System.out.println("Client, 2nd connection: ...connected...");
            XBridge bridge2 = factory.createBridge(
                "", PROTOCOL_DESCRIPTION, connection, prov);
            System.out.println("Client, 2nd connection: ...bridged.");
            synchronized (lock) {
                while (!done) {
                    lock.wait();
                }
            }
            return true;
        }

        private static final class Provider implements XInstanceProvider {
            public Object getInstance(String instanceName) {
                synchronized (map) {
                    Object o = map.get(instanceName);
                    if (o == null) {
                        o = new XDerived() {};
                        map.put(instanceName, o);
                    }
                    return o;
                }
            }

            private final HashMap map = new HashMap();
        }

        private final class Done implements XDone {
            public void notifyDone() {
                synchronized (lock) {
                    done = true;
                    lock.notifyAll();
                }
            }
        }

        private final Object lock = new Object();
        private boolean done = false;
    }

    private static final class Provider implements XInstanceProvider {
        public Provider(TestBed testBed, String unoTypes, String unoServices) {
            this.testBed = testBed;
            this.unoTypes = unoTypes;
            this.unoServices = unoServices;
        }

        public Object getInstance(String instanceName) {
            return new XTestFrame() {
                    public void notifyAccepting(
                        final XDone done, final Object object1,
                        final Object object2)
                    {
                        new Thread() {
                            public void run() {
                                try {
                                    Object test1Aa = object1;
                                    XBase test1Ab = UnoRuntime.queryInterface(
                                        XBase.class, test1Aa);
                                    XDerived test1Ac =
                                        UnoRuntime.queryInterface(
                                            XDerived.class, test1Aa);
                                    Object test2Aa = object2;
                                    XBase test2Ab = UnoRuntime.queryInterface(
                                        XBase.class, test2Aa);
                                    XDerived test2Ac =
                                        UnoRuntime.queryInterface(
                                            XDerived.class, test2Aa);

                                    Hashtable params = new Hashtable();
                                    params.put("UNO_TYPES", unoTypes);
                                    params.put("UNO_SERVICES", unoServices);
                                    XComponentContext context = Bootstrap.
                                       defaultBootstrap_InitialComponentContext(
                                            null, params);
                                    XMultiComponentFactory factory
                                        = context.getServiceManager();
                                    XImplementationLoader loader =
                                        UnoRuntime.queryInterface(
                                            XImplementationLoader.class,
                                            factory.createInstanceWithContext(
                                                "com.sun.star.loader."
                                                + "SharedLibrary",
                                                context));
                                    XSingleComponentFactory factory2 =
                                        UnoRuntime.queryInterface(
                                            XSingleComponentFactory.class,
                                            loader.activate(
                                                "com.sun.star.test.bridges."
                                                + "testequals.impl",
                                                "", "../lib/testequals.uno",
                                                null));
                                    XTestInterface test =
                                        UnoRuntime.queryInterface(
                                            XTestInterface.class,
                                            factory2.createInstanceWithContext(
                                                context));
                                    // allow client to start accepting:
                                    Thread.sleep(3000);
                                    test.connect(
                                        CONNECTION_DESCRIPTION,
                                        PROTOCOL_DESCRIPTION);

                                    Object test1Ba = test.get(INSTANCE1);
                                    XBase test1Bb = UnoRuntime.queryInterface(
                                        XBase.class, test1Ba);
                                    XDerived test1Bc =
                                        UnoRuntime.queryInterface(
                                            XDerived.class, test1Ba);
                                    Object test2Ba = test.get(INSTANCE2);
                                    XBase test2Bb = UnoRuntime.queryInterface(
                                        XBase.class, test2Ba);
                                    XDerived test2Bc =
                                        UnoRuntime.queryInterface(
                                            XDerived.class, test2Ba);

                                    boolean success = true;

                                    success &= test(
                                        "UnoRumtime.areSame(null, null)",
                                        UnoRuntime.areSame(null, null));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test1Aa)",
                                        !UnoRuntime.areSame(null, test1Aa));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test1Ab)",
                                        !UnoRuntime.areSame(null, test1Ab));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test1Ac)",
                                        !UnoRuntime.areSame(null, test1Ac));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test1Ba)",
                                        !UnoRuntime.areSame(null, test1Ba));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test1Bb)",
                                        !UnoRuntime.areSame(null, test1Bb));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test1Bc)",
                                        !UnoRuntime.areSame(null, test1Bc));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test2Aa)",
                                        !UnoRuntime.areSame(null, test2Aa));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test2Ab)",
                                        !UnoRuntime.areSame(null, test2Ab));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test2Ac)",
                                        !UnoRuntime.areSame(null, test2Ac));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test2Ba)",
                                        !UnoRuntime.areSame(null, test2Ba));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test2Bb)",
                                        !UnoRuntime.areSame(null, test2Bb));
                                    success &= test(
                                        "!UnoRumtime.areSame(null, test2Bc)",
                                        !UnoRuntime.areSame(null, test2Bc));

                                    success &= test(
                                        "!test1Aa.equals(null)",
                                        !test1Aa.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Aa, null)",
                                        !UnoRuntime.areSame(test1Aa, null));
                                    success &= test(
                                        "test1Aa.equals(test1Aa)",
                                        test1Aa.equals(test1Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Aa, test1Aa)",
                                        UnoRuntime.areSame(test1Aa, test1Aa));
                                    success &= test(
                                        "test1Aa.equals(test1Ab)",
                                        test1Aa.equals(test1Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Aa, test1Ab)",
                                        UnoRuntime.areSame(test1Aa, test1Ab));
                                    success &= test(
                                        "test1Aa.equals(test1Ac)",
                                        test1Aa.equals(test1Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Aa, test1Ac)",
                                        UnoRuntime.areSame(test1Aa, test1Ac));
                                    success &= test(
                                        "test1Aa.equals(test1Ba)",
                                        test1Aa.equals(test1Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Aa, test1Ba)",
                                        UnoRuntime.areSame(test1Aa, test1Ba));
                                    success &= test(
                                        "test1Aa.equals(test1Bb)",
                                        test1Aa.equals(test1Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Aa, test1Bb)",
                                        UnoRuntime.areSame(test1Aa, test1Bb));
                                    success &= test(
                                        "test1Aa.equals(test1Bc)",
                                        test1Aa.equals(test1Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Aa, test1Bc)",
                                        UnoRuntime.areSame(test1Aa, test1Bc));
                                    success &= test(
                                        "!test1Aa.equals(test2Aa)",
                                        !test1Aa.equals(test2Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Aa, test2Aa)",
                                        !UnoRuntime.areSame(test1Aa, test2Aa));
                                    success &= test(
                                        "!test1Aa.equals(test2Ab)",
                                        !test1Aa.equals(test2Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Aa, test2Ab)",
                                        !UnoRuntime.areSame(test1Aa, test2Ab));
                                    success &= test(
                                        "!test1Aa.equals(test2Ac)",
                                        !test1Aa.equals(test2Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Aa, test2Ac)",
                                        !UnoRuntime.areSame(test1Aa, test2Ac));
                                    success &= test(
                                        "!test1Aa.equals(test2Ba)",
                                        !test1Aa.equals(test2Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Aa, test2Ba)",
                                        !UnoRuntime.areSame(test1Aa, test2Ba));
                                    success &= test(
                                        "!test1Aa.equals(test2Bb)",
                                        !test1Aa.equals(test2Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Aa, test2Bb)",
                                        !UnoRuntime.areSame(test1Aa, test2Bb));
                                    success &= test(
                                        "!test1Aa.equals(test2Bc)",
                                        !test1Aa.equals(test2Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Aa, test2Bc)",
                                        !UnoRuntime.areSame(test1Aa, test2Bc));

                                    success &= test(
                                        "!test1Ab.equals(null)",
                                        !test1Ab.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ab, null)",
                                        !UnoRuntime.areSame(test1Ab, null));
                                    success &= test(
                                        "test1Ab.equals(test1Aa)",
                                        test1Ab.equals(test1Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ab, test1Aa)",
                                        UnoRuntime.areSame(test1Ab, test1Aa));
                                    success &= test(
                                        "test1Ab.equals(test1Ab)",
                                        test1Ab.equals(test1Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ab, test1Ab)",
                                        UnoRuntime.areSame(test1Ab, test1Ab));
                                    success &= test(
                                        "test1Ab.equals(test1Ac)",
                                        test1Ab.equals(test1Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ab, test1Ac)",
                                        UnoRuntime.areSame(test1Ab, test1Ac));
                                    success &= test(
                                        "test1Ab.equals(test1Ba)",
                                        test1Ab.equals(test1Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ab, test1Ba)",
                                        UnoRuntime.areSame(test1Ab, test1Ba));
                                    success &= test(
                                        "test1Ab.equals(test1Bb)",
                                        test1Ab.equals(test1Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ab, test1Bb)",
                                        UnoRuntime.areSame(test1Ab, test1Bb));
                                    success &= test(
                                        "test1Ab.equals(test1Bc)",
                                        test1Ab.equals(test1Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ab, test1Bc)",
                                        UnoRuntime.areSame(test1Ab, test1Bc));
                                    success &= test(
                                        "!test1Ab.equals(test2Aa)",
                                        !test1Ab.equals(test2Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ab, test2Aa)",
                                        !UnoRuntime.areSame(test1Ab, test2Aa));
                                    success &= test(
                                        "!test1Ab.equals(test2Ab)",
                                        !test1Ab.equals(test2Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ab, test2Ab)",
                                        !UnoRuntime.areSame(test1Ab, test2Ab));
                                    success &= test(
                                        "!test1Ab.equals(test2Ac)",
                                        !test1Ab.equals(test2Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ab, test2Ac)",
                                        !UnoRuntime.areSame(test1Ab, test2Ac));
                                    success &= test(
                                        "!test1Ab.equals(test2Ba)",
                                        !test1Ab.equals(test2Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ab, test2Ba)",
                                        !UnoRuntime.areSame(test1Ab, test2Ba));
                                    success &= test(
                                        "!test1Ab.equals(test2Bb)",
                                        !test1Ab.equals(test2Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ab, test2Bb)",
                                        !UnoRuntime.areSame(test1Ab, test2Bb));
                                    success &= test(
                                        "!test1Ab.equals(test2Bc)",
                                        !test1Ab.equals(test2Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ab, test2Bc)",
                                        !UnoRuntime.areSame(test1Ab, test2Bc));

                                    success &= test(
                                        "!test1Ac.equals(null)",
                                        !test1Ac.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ac, null)",
                                        !UnoRuntime.areSame(test1Ac, null));
                                    success &= test(
                                        "test1Ac.equals(test1Aa)",
                                        test1Ac.equals(test1Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ac, test1Aa)",
                                        UnoRuntime.areSame(test1Ac, test1Aa));
                                    success &= test(
                                        "test1Ac.equals(test1Ab)",
                                        test1Ac.equals(test1Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ac, test1Ab)",
                                        UnoRuntime.areSame(test1Ac, test1Ab));
                                    success &= test(
                                        "test1Ac.equals(test1Ac)",
                                        test1Ac.equals(test1Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ac, test1Ac)",
                                        UnoRuntime.areSame(test1Ac, test1Ac));
                                    success &= test(
                                        "test1Ac.equals(test1Ba)",
                                        test1Ac.equals(test1Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ac, test1Ba)",
                                        UnoRuntime.areSame(test1Ac, test1Ba));
                                    success &= test(
                                        "test1Ac.equals(test1Bb)",
                                        test1Ac.equals(test1Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ac, test1Bb)",
                                        UnoRuntime.areSame(test1Ac, test1Bb));
                                    success &= test(
                                        "test1Ac.equals(test1Bc)",
                                        test1Ac.equals(test1Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ac, test1Bc)",
                                        UnoRuntime.areSame(test1Ac, test1Bc));
                                    success &= test(
                                        "!test1Ac.equals(test2Aa)",
                                        !test1Ac.equals(test2Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ac, test2Aa)",
                                        !UnoRuntime.areSame(test1Ac, test2Aa));
                                    success &= test(
                                        "!test1Ac.equals(test2Ab)",
                                        !test1Ac.equals(test2Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ac, test2Ab)",
                                        !UnoRuntime.areSame(test1Ac, test2Ab));
                                    success &= test(
                                        "!test1Ac.equals(test2Ac)",
                                        !test1Ac.equals(test2Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ac, test2Ac)",
                                        !UnoRuntime.areSame(test1Ac, test2Ac));
                                    success &= test(
                                        "!test1Ac.equals(test2Ba)",
                                        !test1Ac.equals(test2Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ac, test2Ba)",
                                        !UnoRuntime.areSame(test1Ac, test2Ba));
                                    success &= test(
                                        "!test1Ac.equals(test2Bb)",
                                        !test1Ac.equals(test2Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ac, test2Bb)",
                                        !UnoRuntime.areSame(test1Ac, test2Bb));
                                    success &= test(
                                        "!test1Ac.equals(test2Bc)",
                                        !test1Ac.equals(test2Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ac, test2Bc)",
                                        !UnoRuntime.areSame(test1Ac, test2Bc));

                                    success &= test(
                                        "!test1Ba.equals(null)",
                                        !test1Ba.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ba, null)",
                                        !UnoRuntime.areSame(test1Ba, null));
                                    success &= test(
                                        "test1Ba.equals(test1Aa)",
                                        test1Ba.equals(test1Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ba, test1Aa)",
                                        UnoRuntime.areSame(test1Ba, test1Aa));
                                    success &= test(
                                        "test1Ba.equals(test1Ab)",
                                        test1Ba.equals(test1Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ba, test1Ab)",
                                        UnoRuntime.areSame(test1Ba, test1Ab));
                                    success &= test(
                                        "test1Ba.equals(test1Ac)",
                                        test1Ba.equals(test1Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ba, test1Ac)",
                                        UnoRuntime.areSame(test1Ba, test1Ac));
                                    success &= test(
                                        "test1Ba.equals(test1Ba)",
                                        test1Ba.equals(test1Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ba, test1Ba)",
                                        UnoRuntime.areSame(test1Ba, test1Ba));
                                    success &= test(
                                        "test1Ba.equals(test1Bb)",
                                        test1Ba.equals(test1Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ba, test1Bb)",
                                        UnoRuntime.areSame(test1Ba, test1Bb));
                                    success &= test(
                                        "test1Ba.equals(test1Bc)",
                                        test1Ba.equals(test1Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Ba, test1Bc)",
                                        UnoRuntime.areSame(test1Ba, test1Bc));
                                    success &= test(
                                        "!test1Ba.equals(test2Aa)",
                                        !test1Ba.equals(test2Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ba, test2Aa)",
                                        !UnoRuntime.areSame(test1Ba, test2Aa));
                                    success &= test(
                                        "!test1Ba.equals(test2Ab)",
                                        !test1Ba.equals(test2Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ba, test2Ab)",
                                        !UnoRuntime.areSame(test1Ba, test2Ab));
                                    success &= test(
                                        "!test1Ba.equals(test2Ac)",
                                        !test1Ba.equals(test2Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ba, test2Ac)",
                                        !UnoRuntime.areSame(test1Ba, test2Ac));
                                    success &= test(
                                        "!test1Ba.equals(test2Ba)",
                                        !test1Ba.equals(test2Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ba, test2Ba)",
                                        !UnoRuntime.areSame(test1Ba, test2Ba));
                                    success &= test(
                                        "!test1Ba.equals(test2Bb)",
                                        !test1Ba.equals(test2Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ba, test2Bb)",
                                        !UnoRuntime.areSame(test1Ba, test2Bb));
                                    success &= test(
                                        "!test1Ba.equals(test2Bc)",
                                        !test1Ba.equals(test2Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Ba, test2Bc)",
                                        !UnoRuntime.areSame(test1Ba, test2Bc));

                                    success &= test(
                                        "!test1Bb.equals(null)",
                                        !test1Bb.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bb, null)",
                                        !UnoRuntime.areSame(test1Bb, null));
                                    success &= test(
                                        "test1Bb.equals(test1Aa)",
                                        test1Bb.equals(test1Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bb, test1Aa)",
                                        UnoRuntime.areSame(test1Bb, test1Aa));
                                    success &= test(
                                        "test1Bb.equals(test1Ab)",
                                        test1Bb.equals(test1Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bb, test1Ab)",
                                        UnoRuntime.areSame(test1Bb, test1Ab));
                                    success &= test(
                                        "test1Bb.equals(test1Ac)",
                                        test1Bb.equals(test1Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bb, test1Ac)",
                                        UnoRuntime.areSame(test1Bb, test1Ac));
                                    success &= test(
                                        "test1Bb.equals(test1Ba)",
                                        test1Bb.equals(test1Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bb, test1Ba)",
                                        UnoRuntime.areSame(test1Bb, test1Ba));
                                    success &= test(
                                        "test1Bb.equals(test1Bb)",
                                        test1Bb.equals(test1Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bb, test1Bb)",
                                        UnoRuntime.areSame(test1Bb, test1Bb));
                                    success &= test(
                                        "test1Bb.equals(test1Bc)",
                                        test1Bb.equals(test1Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bb, test1Bc)",
                                        UnoRuntime.areSame(test1Bb, test1Bc));
                                    success &= test(
                                        "!test1Bb.equals(test2Aa)",
                                        !test1Bb.equals(test2Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bb, test2Aa)",
                                        !UnoRuntime.areSame(test1Bb, test2Aa));
                                    success &= test(
                                        "!test1Bb.equals(test2Ab)",
                                        !test1Bb.equals(test2Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bb, test2Ab)",
                                        !UnoRuntime.areSame(test1Bb, test2Ab));
                                    success &= test(
                                        "!test1Bb.equals(test2Ac)",
                                        !test1Bb.equals(test2Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bb, test2Ac)",
                                        !UnoRuntime.areSame(test1Bb, test2Ac));
                                    success &= test(
                                        "!test1Bb.equals(test2Ba)",
                                        !test1Bb.equals(test2Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bb, test2Ba)",
                                        !UnoRuntime.areSame(test1Bb, test2Ba));
                                    success &= test(
                                        "!test1Bb.equals(test2Bb)",
                                        !test1Bb.equals(test2Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bb, test2Bb)",
                                        !UnoRuntime.areSame(test1Bb, test2Bb));
                                    success &= test(
                                        "!test1Bb.equals(test2Bc)",
                                        !test1Bb.equals(test2Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bb, test2Bc)",
                                        !UnoRuntime.areSame(test1Bb, test2Bc));

                                    success &= test(
                                        "!test1Bc.equals(null)",
                                        !test1Bc.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bc, null)",
                                        !UnoRuntime.areSame(test1Bc, null));
                                    success &= test(
                                        "test1Bc.equals(test1Aa)",
                                        test1Bc.equals(test1Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bc, test1Aa)",
                                        UnoRuntime.areSame(test1Bc, test1Aa));
                                    success &= test(
                                        "test1Bc.equals(test1Ab)",
                                        test1Bc.equals(test1Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bc, test1Ab)",
                                        UnoRuntime.areSame(test1Bc, test1Ab));
                                    success &= test(
                                        "test1Bc.equals(test1Ac)",
                                        test1Bc.equals(test1Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bc, test1Ac)",
                                        UnoRuntime.areSame(test1Bc, test1Ac));
                                    success &= test(
                                        "test1Bc.equals(test1Ba)",
                                        test1Bc.equals(test1Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bc, test1Ba)",
                                        UnoRuntime.areSame(test1Bc, test1Ba));
                                    success &= test(
                                        "test1Bc.equals(test1Bb)",
                                        test1Bc.equals(test1Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bc, test1Bb)",
                                        UnoRuntime.areSame(test1Bc, test1Bb));
                                    success &= test(
                                        "test1Bc.equals(test1Bc)",
                                        test1Bc.equals(test1Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test1Bc, test1Bc)",
                                        UnoRuntime.areSame(test1Bc, test1Bc));
                                    success &= test(
                                        "!test1Bc.equals(test2Aa)",
                                        !test1Bc.equals(test2Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bc, test2Aa)",
                                        !UnoRuntime.areSame(test1Bc, test2Aa));
                                    success &= test(
                                        "!test1Bc.equals(test2Ab)",
                                        !test1Bc.equals(test2Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bc, test2Ab)",
                                        !UnoRuntime.areSame(test1Bc, test2Ab));
                                    success &= test(
                                        "!test1Bc.equals(test2Ac)",
                                        !test1Bc.equals(test2Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bc, test2Ac)",
                                        !UnoRuntime.areSame(test1Bc, test2Ac));
                                    success &= test(
                                        "!test1Bc.equals(test2Ba)",
                                        !test1Bc.equals(test2Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bc, test2Ba)",
                                        !UnoRuntime.areSame(test1Bc, test2Ba));
                                    success &= test(
                                        "!test1Bc.equals(test2Bb)",
                                        !test1Bc.equals(test2Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bc, test2Bb)",
                                        !UnoRuntime.areSame(test1Bc, test2Bb));
                                    success &= test(
                                        "!test1Bc.equals(test2Bc)",
                                        !test1Bc.equals(test2Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test1Bc, test2Bc)",
                                        !UnoRuntime.areSame(test1Bc, test2Bc));

                                    success &= test(
                                        "!test2Aa.equals(null)",
                                        !test2Aa.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Aa, null)",
                                        !UnoRuntime.areSame(test2Aa, null));
                                    success &= test(
                                        "!test2Aa.equals(test1Aa)",
                                        !test2Aa.equals(test1Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Aa, test1Aa)",
                                        !UnoRuntime.areSame(test2Aa, test1Aa));
                                    success &= test(
                                        "!test2Aa.equals(test1Ab)",
                                        !test2Aa.equals(test1Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Aa, test1Ab)",
                                        !UnoRuntime.areSame(test2Aa, test1Ab));
                                    success &= test(
                                        "!test2Aa.equals(test1Ac)",
                                        !test2Aa.equals(test1Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Aa, test1Ac)",
                                        !UnoRuntime.areSame(test2Aa, test1Ac));
                                    success &= test(
                                        "!test2Aa.equals(test1Ba)",
                                        !test2Aa.equals(test1Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Aa, test1Ba)",
                                        !UnoRuntime.areSame(test2Aa, test1Ba));
                                    success &= test(
                                        "!test2Aa.equals(test1Bb)",
                                        !test2Aa.equals(test1Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Aa, test1Bb)",
                                        !UnoRuntime.areSame(test2Aa, test1Bb));
                                    success &= test(
                                        "!test2Aa.equals(test1Bc)",
                                        !test2Aa.equals(test1Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Aa, test1Bc)",
                                        !UnoRuntime.areSame(test2Aa, test1Bc));
                                    success &= test(
                                        "test2Aa.equals(test2Aa)",
                                        test2Aa.equals(test2Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Aa, test2Aa)",
                                        UnoRuntime.areSame(test2Aa, test2Aa));
                                    success &= test(
                                        "test2Aa.equals(test2Ab)",
                                        test2Aa.equals(test2Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Aa, test2Ab)",
                                        UnoRuntime.areSame(test2Aa, test2Ab));
                                    success &= test(
                                        "test2Aa.equals(test2Ac)",
                                        test2Aa.equals(test2Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Aa, test2Ac)",
                                        UnoRuntime.areSame(test2Aa, test2Ac));
                                    success &= test(
                                        "test2Aa.equals(test2Ba)",
                                        test2Aa.equals(test2Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Aa, test2Ba)",
                                        UnoRuntime.areSame(test2Aa, test2Ba));
                                    success &= test(
                                        "test2Aa.equals(test2Bb)",
                                        test2Aa.equals(test2Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Aa, test2Bb)",
                                        UnoRuntime.areSame(test2Aa, test2Bb));
                                    success &= test(
                                        "test2Aa.equals(test2Bc)",
                                        test2Aa.equals(test2Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Aa, test2Bc)",
                                        UnoRuntime.areSame(test2Aa, test2Bc));

                                    success &= test(
                                        "!test2Ab.equals(null)",
                                        !test2Ab.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ab, null)",
                                        !UnoRuntime.areSame(test2Ab, null));
                                    success &= test(
                                        "!test2Ab.equals(test1Aa)",
                                        !test2Ab.equals(test1Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ab, test1Aa)",
                                        !UnoRuntime.areSame(test2Ab, test1Aa));
                                    success &= test(
                                        "!test2Ab.equals(test1Ab)",
                                        !test2Ab.equals(test1Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ab, test1Ab)",
                                        !UnoRuntime.areSame(test2Ab, test1Ab));
                                    success &= test(
                                        "!test2Ab.equals(test1Ac)",
                                        !test2Ab.equals(test1Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ab, test1Ac)",
                                        !UnoRuntime.areSame(test2Ab, test1Ac));
                                    success &= test(
                                        "!test2Ab.equals(test1Ba)",
                                        !test2Ab.equals(test1Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ab, test1Ba)",
                                        !UnoRuntime.areSame(test2Ab, test1Ba));
                                    success &= test(
                                        "!test2Ab.equals(test1Bb)",
                                        !test2Ab.equals(test1Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ab, test1Bb)",
                                        !UnoRuntime.areSame(test2Ab, test1Bb));
                                    success &= test(
                                        "!test2Ab.equals(test1Bc)",
                                        !test2Ab.equals(test1Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ab, test1Bc)",
                                        !UnoRuntime.areSame(test2Ab, test1Bc));
                                    success &= test(
                                        "test2Ab.equals(test2Aa)",
                                        test2Ab.equals(test2Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ab, test2Aa)",
                                        UnoRuntime.areSame(test2Ab, test2Aa));
                                    success &= test(
                                        "test2Ab.equals(test2Ab)",
                                        test2Ab.equals(test2Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ab, test2Ab)",
                                        UnoRuntime.areSame(test2Ab, test2Ab));
                                    success &= test(
                                        "test2Ab.equals(test2Ac)",
                                        test2Ab.equals(test2Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ab, test2Ac)",
                                        UnoRuntime.areSame(test2Ab, test2Ac));
                                    success &= test(
                                        "test2Ab.equals(test2Ba)",
                                        test2Ab.equals(test2Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ab, test2Ba)",
                                        UnoRuntime.areSame(test2Ab, test2Ba));
                                    success &= test(
                                        "test2Ab.equals(test2Bb)",
                                        test2Ab.equals(test2Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ab, test2Bb)",
                                        UnoRuntime.areSame(test2Ab, test2Bb));
                                    success &= test(
                                        "test2Ab.equals(test2Bc)",
                                        test2Ab.equals(test2Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ab, test2Bc)",
                                        UnoRuntime.areSame(test2Ab, test2Bc));

                                    success &= test(
                                        "!test2Ac.equals(null)",
                                        !test2Ac.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ac, null)",
                                        !UnoRuntime.areSame(test2Ac, null));
                                    success &= test(
                                        "!test2Ac.equals(test1Aa)",
                                        !test2Ac.equals(test1Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ac, test1Aa)",
                                        !UnoRuntime.areSame(test2Ac, test1Aa));
                                    success &= test(
                                        "!test2Ac.equals(test1Ab)",
                                        !test2Ac.equals(test1Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ac, test1Ab)",
                                        !UnoRuntime.areSame(test2Ac, test1Ab));
                                    success &= test(
                                        "!test2Ac.equals(test1Ac)",
                                        !test2Ac.equals(test1Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ac, test1Ac)",
                                        !UnoRuntime.areSame(test2Ac, test1Ac));
                                    success &= test(
                                        "!test2Ac.equals(test1Ba)",
                                        !test2Ac.equals(test1Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ac, test1Ba)",
                                        !UnoRuntime.areSame(test2Ac, test1Ba));
                                    success &= test(
                                        "!test2Ac.equals(test1Bb)",
                                        !test2Ac.equals(test1Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ac, test1Bb)",
                                        !UnoRuntime.areSame(test2Ac, test1Bb));
                                    success &= test(
                                        "!test2Ac.equals(test1Bc)",
                                        !test2Ac.equals(test1Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ac, test1Bc)",
                                        !UnoRuntime.areSame(test2Ac, test1Bc));
                                    success &= test(
                                        "test2Ac.equals(test2Aa)",
                                        test2Ac.equals(test2Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ac, test2Aa)",
                                        UnoRuntime.areSame(test2Ac, test2Aa));
                                    success &= test(
                                        "test2Ac.equals(test2Ab)",
                                        test2Ac.equals(test2Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ac, test2Ab)",
                                        UnoRuntime.areSame(test2Ac, test2Ab));
                                    success &= test(
                                        "test2Ac.equals(test2Ac)",
                                        test2Ac.equals(test2Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ac, test2Ac)",
                                        UnoRuntime.areSame(test2Ac, test2Ac));
                                    success &= test(
                                        "test2Ac.equals(test2Ba)",
                                        test2Ac.equals(test2Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ac, test2Ba)",
                                        UnoRuntime.areSame(test2Ac, test2Ba));
                                    success &= test(
                                        "test2Ac.equals(test2Bb)",
                                        test2Ac.equals(test2Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ac, test2Bb)",
                                        UnoRuntime.areSame(test2Ac, test2Bb));
                                    success &= test(
                                        "test2Ac.equals(test2Bc)",
                                        test2Ac.equals(test2Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ac, test2Bc)",
                                        UnoRuntime.areSame(test2Ac, test2Bc));

                                    success &= test(
                                        "!test2Ba.equals(null)",
                                        !test2Ba.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ba, null)",
                                        !UnoRuntime.areSame(test2Ba, null));
                                    success &= test(
                                        "!test2Ba.equals(test1Aa)",
                                        !test2Ba.equals(test1Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ba, test1Aa)",
                                        !UnoRuntime.areSame(test2Ba, test1Aa));
                                    success &= test(
                                        "!test2Ba.equals(test1Ab)",
                                        !test2Ba.equals(test1Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ba, test1Ab)",
                                        !UnoRuntime.areSame(test2Ba, test1Ab));
                                    success &= test(
                                        "!test2Ba.equals(test1Ac)",
                                        !test2Ba.equals(test1Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ba, test1Ac)",
                                        !UnoRuntime.areSame(test2Ba, test1Ac));
                                    success &= test(
                                        "!test2Ba.equals(test1Ba)",
                                        !test2Ba.equals(test1Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ba, test1Ba)",
                                        !UnoRuntime.areSame(test2Ba, test1Ba));
                                    success &= test(
                                        "!test2Ba.equals(test1Bb)",
                                        !test2Ba.equals(test1Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ba, test1Bb)",
                                        !UnoRuntime.areSame(test2Ba, test1Bb));
                                    success &= test(
                                        "!test2Ba.equals(test1Bc)",
                                        !test2Ba.equals(test1Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Ba, test1Bc)",
                                        !UnoRuntime.areSame(test2Ba, test1Bc));
                                    success &= test(
                                        "test2Ba.equals(test2Aa)",
                                        test2Ba.equals(test2Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ba, test2Aa)",
                                        UnoRuntime.areSame(test2Ba, test2Aa));
                                    success &= test(
                                        "test2Ba.equals(test2Ab)",
                                        test2Ba.equals(test2Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ba, test2Ab)",
                                        UnoRuntime.areSame(test2Ba, test2Ab));
                                    success &= test(
                                        "test2Ba.equals(test2Ac)",
                                        test2Ba.equals(test2Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ba, test2Ac)",
                                        UnoRuntime.areSame(test2Ba, test2Ac));
                                    success &= test(
                                        "test2Ba.equals(test2Ba)",
                                        test2Ba.equals(test2Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ba, test2Ba)",
                                        UnoRuntime.areSame(test2Ba, test2Ba));
                                    success &= test(
                                        "test2Ba.equals(test2Bb)",
                                        test2Ba.equals(test2Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ba, test2Bb)",
                                        UnoRuntime.areSame(test2Ba, test2Bb));
                                    success &= test(
                                        "test2Ba.equals(test2Bc)",
                                        test2Ba.equals(test2Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Ba, test2Bc)",
                                        UnoRuntime.areSame(test2Ba, test2Bc));

                                    success &= test(
                                        "!test2Bb.equals(null)",
                                        !test2Bb.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bb, null)",
                                        !UnoRuntime.areSame(test2Bb, null));
                                    success &= test(
                                        "!test2Bb.equals(test1Aa)",
                                        !test2Bb.equals(test1Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bb, test1Aa)",
                                        !UnoRuntime.areSame(test2Bb, test1Aa));
                                    success &= test(
                                        "!test2Bb.equals(test1Ab)",
                                        !test2Bb.equals(test1Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bb, test1Ab)",
                                        !UnoRuntime.areSame(test2Bb, test1Ab));
                                    success &= test(
                                        "!test2Bb.equals(test1Ac)",
                                        !test2Bb.equals(test1Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bb, test1Ac)",
                                        !UnoRuntime.areSame(test2Bb, test1Ac));
                                    success &= test(
                                        "!test2Bb.equals(test1Ba)",
                                        !test2Bb.equals(test1Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bb, test1Ba)",
                                        !UnoRuntime.areSame(test2Bb, test1Ba));
                                    success &= test(
                                        "!test2Bb.equals(test1Bb)",
                                        !test2Bb.equals(test1Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bb, test1Bb)",
                                        !UnoRuntime.areSame(test2Bb, test1Bb));
                                    success &= test(
                                        "!test2Bb.equals(test1Bc)",
                                        !test2Bb.equals(test1Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bb, test1Bc)",
                                        !UnoRuntime.areSame(test2Bb, test1Bc));
                                    success &= test(
                                        "test2Bb.equals(test2Aa)",
                                        test2Bb.equals(test2Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bb, test2Aa)",
                                        UnoRuntime.areSame(test2Bb, test2Aa));
                                    success &= test(
                                        "test2Bb.equals(test2Ab)",
                                        test2Bb.equals(test2Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bb, test2Ab)",
                                        UnoRuntime.areSame(test2Bb, test2Ab));
                                    success &= test(
                                        "test2Bb.equals(test2Ac)",
                                        test2Bb.equals(test2Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bb, test2Ac)",
                                        UnoRuntime.areSame(test2Bb, test2Ac));
                                    success &= test(
                                        "test2Bb.equals(test2Ba)",
                                        test2Bb.equals(test2Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bb, test2Ba)",
                                        UnoRuntime.areSame(test2Bb, test2Ba));
                                    success &= test(
                                        "test2Bb.equals(test2Bb)",
                                        test2Bb.equals(test2Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bb, test2Bb)",
                                        UnoRuntime.areSame(test2Bb, test2Bb));
                                    success &= test(
                                        "test2Bb.equals(test2Bc)",
                                        test2Bb.equals(test2Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bb, test2Bc)",
                                        UnoRuntime.areSame(test2Bb, test2Bc));

                                    success &= test(
                                        "!test2Bc.equals(null)",
                                        !test2Bc.equals(null));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bc, null)",
                                        !UnoRuntime.areSame(test2Bc, null));
                                    success &= test(
                                        "!test2Bc.equals(test1Aa)",
                                        !test2Bc.equals(test1Aa));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bc, test1Aa)",
                                        !UnoRuntime.areSame(test2Bc, test1Aa));
                                    success &= test(
                                        "!test2Bc.equals(test1Ab)",
                                        !test2Bc.equals(test1Ab));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bc, test1Ab)",
                                        !UnoRuntime.areSame(test2Bc, test1Ab));
                                    success &= test(
                                        "!test2Bc.equals(test1Ac)",
                                        !test2Bc.equals(test1Ac));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bc, test1Ac)",
                                        !UnoRuntime.areSame(test2Bc, test1Ac));
                                    success &= test(
                                        "!test2Bc.equals(test1Ba)",
                                        !test2Bc.equals(test1Ba));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bc, test1Ba)",
                                        !UnoRuntime.areSame(test2Bc, test1Ba));
                                    success &= test(
                                        "!test2Bc.equals(test1Bb)",
                                        !test2Bc.equals(test1Bb));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bc, test1Bb)",
                                        !UnoRuntime.areSame(test2Bc, test1Bb));
                                    success &= test(
                                        "!test2Bc.equals(test1Bc)",
                                        !test2Bc.equals(test1Bc));
                                    success &= test(
                                        "!UnoRuntime.areSame(test2Bc, test1Bc)",
                                        !UnoRuntime.areSame(test2Bc, test1Bc));
                                    success &= test(
                                        "test2Bc.equals(test2Aa)",
                                        test2Bc.equals(test2Aa));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bc, test2Aa)",
                                        UnoRuntime.areSame(test2Bc, test2Aa));
                                    success &= test(
                                        "test2Bc.equals(test2Ab)",
                                        test2Bc.equals(test2Ab));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bc, test2Ab)",
                                        UnoRuntime.areSame(test2Bc, test2Ab));
                                    success &= test(
                                        "test2Bc.equals(test2Ac)",
                                        test2Bc.equals(test2Ac));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bc, test2Ac)",
                                        UnoRuntime.areSame(test2Bc, test2Ac));
                                    success &= test(
                                        "test2Bc.equals(test2Ba)",
                                        test2Bc.equals(test2Ba));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bc, test2Ba)",
                                        UnoRuntime.areSame(test2Bc, test2Ba));
                                    success &= test(
                                        "test2Bc.equals(test2Bb)",
                                        test2Bc.equals(test2Bb));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bc, test2Bb)",
                                        UnoRuntime.areSame(test2Bc, test2Bb));
                                    success &= test(
                                        "test2Bc.equals(test2Bc)",
                                        test2Bc.equals(test2Bc));
                                    success &= test(
                                        "UnoRuntime.areSame(test2Bc, test2Bc)",
                                        UnoRuntime.areSame(test2Bc, test2Bc));

                                    success &= test(
                                        "test1Aa.hashCode() == test1Ab.hashCode()",
                                        test1Aa.hashCode()
                                        == test1Ab.hashCode());
                                    success &= test(
                                        "test1Aa.hashCode()"
                                        + " == test1Ac.hashCode()",
                                        test1Aa.hashCode()
                                        == test1Ac.hashCode());
                                    success &= test(
                                        "test1Aa.hashCode()"
                                        + " == test1Ba.hashCode()",
                                        test1Aa.hashCode()
                                        == test1Ba.hashCode());
                                    success &= test(
                                        "test1Aa.hashCode()"
                                        + " == test1Bb.hashCode()",
                                        test1Aa.hashCode()
                                        == test1Bb.hashCode());
                                    success &= test(
                                        "test1Aa.hashCode()"
                                        + " == test1Bc.hashCode()",
                                        test1Aa.hashCode()
                                        == test1Bc.hashCode());
                                    success &= test(
                                        "test2Aa.hashCode()"
                                        + " == test2Ab.hashCode()",
                                        test2Aa.hashCode()
                                        == test2Ab.hashCode());
                                    success &= test(
                                        "test2Aa.hashCode()"
                                        + " == test2Ac.hashCode()",
                                        test2Aa.hashCode()
                                        == test2Ac.hashCode());
                                    success &= test(
                                        "test2Aa.hashCode()"
                                        + " == test2Ba.hashCode()",
                                        test2Aa.hashCode()
                                        == test2Ba.hashCode());
                                    success &= test(
                                        "test2Aa.hashCode()"
                                        + " == test2Bb.hashCode()",
                                        test2Aa.hashCode()
                                        == test2Bb.hashCode());
                                    success &= test(
                                        "test2Aa.hashCode()"
                                        + " == test2Bc.hashCode()",
                                        test2Aa.hashCode()
                                        == test2Bc.hashCode());

                                    done.notifyDone();
                                    testBed.serverDone(success);
                                } catch (Exception e) {
                                    e.printStackTrace(System.err);
                                }
                            };

                            private /*static*/ boolean test(
                                String message, boolean condition)
                            {
                                if (!condition) {
                                    System.err.println("Failed: " + message);
                                }
                                return condition;
                            }
                        }.start();
                    }
                };
        }

        private final TestBed testBed;
        private final String unoTypes;
        private final String unoServices;
    }

    public interface XDone extends XInterface {
        void notifyDone();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("notifyDone", 0, 0) };
    }

    public interface XTestFrame extends XInterface {
        void notifyAccepting(XDone done, Object object1, Object object2);

        TypeInfo[] UNOTYPEINFO = {
            new MethodTypeInfo("notifyAccepting", 0, TypeInfo.ONEWAY) };
    }

    // Use "127.0.0.1" instead of "localhost", see #i32281#:
    private static final String CONNECTION_DESCRIPTION
    = "socket,host=127.0.0.1,port=12346";
    private static final String PROTOCOL_DESCRIPTION = "urp";

    private static final String INSTANCE1 = "instance1";
    private static final String INSTANCE2 = "instance2";
}
