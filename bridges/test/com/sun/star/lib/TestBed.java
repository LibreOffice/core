/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestBed.java,v $
 * $Revision: 1.7 $
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

package com.sun.star.lib;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.connection.Acceptor;
import com.sun.star.connection.Connector;
import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;

public final class TestBed {
    public boolean execute(XInstanceProvider provider, boolean waitForServer,
                           Class client, long wait) throws Exception {
        // assert client.isAssignableFrom(client) && wait >= 0;
        synchronized (lock) {
            server = new Server(provider);
            server.start();
            server.waitAccepting();
        }
        Process p = Runtime.getRuntime().exec(new String[] {
            "java", "-classpath", System.getProperty("java.class.path"),
/*
            "-Xdebug",
            "-Xrunjdwp:transport=dt_socket,address=8000,server=y,suspend=n",
*/
            client.getName() });
        pipe(p.getInputStream(), System.out, "CO> ");
        pipe(p.getErrorStream(), System.err, "CE> ");
        boolean clientDone = false;
        if (wait <= 0) {
            clientDone = p.waitFor() == CLIENT_DONE;
        } else {
            try {
                Thread.sleep(wait);
            } catch (InterruptedException e) {
                p.destroy();
                throw e;
            }
            try {
                clientDone = p.exitValue() == CLIENT_DONE;
            } catch (IllegalThreadStateException e) {
                p.destroy();
            }
        }
        boolean success = clientDone;
        if (waitForServer) {
            success &= server.waitDone();
        }
        return success;
    }

    public void serverDone(boolean success) {
        synchronized (lock) {
            server.done(success);
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

    public static abstract class Client {
        protected abstract boolean run(XComponentContext context)
            throws Throwable;

        protected final String getConnectionDescription() {
            return connectionDescription;
        }

        protected final String getProtocolDescription() {
            return protocolDescription;
        }

        protected final XBridge getBridge(XComponentContext context)
            throws com.sun.star.uno.Exception
        {
            XConnector connector = Connector.create(context);
            XBridgeFactory factory = UnoRuntime.queryInterface(
                XBridgeFactory.class,
                context.getServiceManager().createInstanceWithContext(
                    "com.sun.star.bridge.BridgeFactory", context));
            System.out.println("Client: Connecting...");
            XConnection connection = connector.connect(connectionDescription);
            System.out.println("Client: ...connected...");
            XBridge bridge = factory.createBridge(
                "", protocolDescription, connection, null);
            System.out.println("Client: ...bridged.");
            return bridge;
        }

        protected final void execute() {
            int status = CLIENT_FAILED;
            try {
                if (run(Bootstrap.createInitialComponentContext(null))) {
                    status = CLIENT_DONE;
                }
            } catch (Throwable e) {
                e.printStackTrace(System.err);
            }
            System.exit(status);
        }
    }

    private static final class Server extends Thread {
        public Server(XInstanceProvider provider) {
            super("Server");
            // assert provider != null;
            this.provider = provider;
        }

        public void run() {
            try {
                XComponentContext context
                    = Bootstrap.createInitialComponentContext(null);
                XAcceptor acceptor = Acceptor.create(context);
                XBridgeFactory factory
                    = UnoRuntime.queryInterface(
                        XBridgeFactory.class,
                        context.getServiceManager().createInstanceWithContext(
                            "com.sun.star.bridge.BridgeFactory", context));
                System.out.println("Server: Accepting...");
                synchronized (this) {
                    state = ACCEPTING;
                    notifyAll();
                }
                for (;;) {
                    XConnection connection = acceptor.accept(
                        connectionDescription);
                    System.out.println("Server: ...connected...");
                    XBridge bridge = factory.createBridge(
                        "", protocolDescription, connection, provider);
                    System.out.println("Server: ...bridged.");
                }
            } catch (Throwable e) {
                e.printStackTrace(System.err);
            }
        }

        public synchronized void waitAccepting() throws InterruptedException {
            while (state < ACCEPTING) {
                wait();
            }
        }

        public synchronized boolean waitDone() throws InterruptedException {
            while (state <= ACCEPTING) {
                wait();
            }
            return state == SUCCEEDED;
        }

        public synchronized void done(boolean success) {
            state = success ? SUCCEEDED : FAILED;
            notifyAll();
        }

        private static final int INITIAL = 0;
        private static final int ACCEPTING = 1;
        private static final int FAILED = 2;
        private static final int SUCCEEDED = 3;

        private final XInstanceProvider provider;

        private int state = INITIAL;
    }

    private static final int TEST_SUCCEEDED = 0;
    private static final int TEST_FAILED = 1;
    private static final int TEST_ERROR = 2;

    private static final int CLIENT_FAILED = 0;
    private static final int CLIENT_DONE = 123;

    private static final String connectionDescription
    = "socket,host=localhost,port=12345";
    private static final String protocolDescription = "urp";

    private final Object lock = new Object();
    private Server server = null;
}
