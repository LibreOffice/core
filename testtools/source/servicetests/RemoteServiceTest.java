/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RemoteServiceTest.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:28:49 $
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

package testtools.servicetests;

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
import complexlib.ComplexTestCase;
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
            ((XSet) UnoRuntime.queryInterface(XSet.class, serviceManager)).
                insert(new TestService());
            final Object instance = serviceManager.createInstanceWithContext(
                "testtools.servicetests.TestService2", context);
            XBridgeFactory bridgeFactory
                = (XBridgeFactory) UnoRuntime.queryInterface(
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
