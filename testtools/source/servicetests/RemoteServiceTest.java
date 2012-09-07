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

package servicetests;

import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.connection.Acceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.container.XSet;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;

public final class RemoteServiceTest extends TestBase {
    protected TestServiceFactory getTestServiceFactory() throws Exception {
        final Process p = Runtime.getRuntime().exec(new String[] {
            "java", "-classpath", System.getProperty("java.class.path"),
            Server.class.getName() });
        pipe(p.getInputStream(), System.out, "CO> ");
        pipe(p.getErrorStream(), System.err, "CE> ");
        Thread.sleep(5000); // wait for server to start accepting
        return new TestServiceFactory() {
                public Object get() throws Exception {
                    return (UnoUrlResolver.create(
                                Bootstrap.createInitialComponentContext(null))).
                        resolve(
                            "uno:" + CONNECTION_DESCRIPTION + ";"
                            + PROTOCOL_DESCRIPTION
                            + ";testtools.servicetests.TestService2");
                }

                public void dispose() throws Exception {
                    p.waitFor();
                }
            };
    }

    public static final class Server {
        public static void main(String[] arguments) throws Exception {
            XComponentContext context
                = Bootstrap.createInitialComponentContext(null);
            XMultiComponentFactory serviceManager
                = context.getServiceManager();
            UnoRuntime.queryInterface(XSet.class, serviceManager).
                insert(new TestService());
            final Object instance = serviceManager.createInstanceWithContext(
                "testtools.servicetests.TestService2", context);
            XBridgeFactory bridgeFactory
                = UnoRuntime.queryInterface(
                    XBridgeFactory.class,
                    serviceManager.createInstanceWithContext(
                        "com.sun.star.bridge.BridgeFactory", context));
            XConnection connection = Acceptor.create(context).accept(
                CONNECTION_DESCRIPTION);
            bridgeFactory.createBridge(
                "", PROTOCOL_DESCRIPTION, connection,
                new XInstanceProvider() {
                    public Object getInstance(String instanceName) {
                        return instance;
                    }
                });
        }
    }

    private void pipe(final InputStream in, final PrintStream out,
                      final String prefix) {
        new Thread("Pipe: " + prefix) {
            public void run() {
                BufferedReader r
                    = new BufferedReader(new InputStreamReader(in));
                try {
                    for (;;) {
                        String s = r.readLine();
                        if (s == null) {
                            break;
                        }
                        out.println(prefix + s);
                    }
                } catch (java.io.IOException e) {
                    e.printStackTrace(System.err);
                }
            }
        }.start();
    }

    private static final String CONNECTION_DESCRIPTION
    = "socket,host=localhost,port=12345";
    private static final String PROTOCOL_DESCRIPTION = "urp";
}
