/*************************************************************************
 *
 *  $RCSfile: TestBed.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:49:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        protected abstract boolean run(XBridge bridge) throws Throwable;

        protected final XBridge getBridge() throws com.sun.star.uno.Exception {
            XConnector connector = Connector.create(context);
            XBridgeFactory factory = (XBridgeFactory) UnoRuntime.queryInterface(
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
                context = Bootstrap.createInitialComponentContext(null);
                if (run(getBridge())) {
                    status = CLIENT_DONE;
                }
            } catch (Throwable e) {
                e.printStackTrace(System.err);
            }
            System.exit(status);
        }

        private XComponentContext context;
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
                    = (XBridgeFactory) UnoRuntime.queryInterface(
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
