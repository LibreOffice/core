/*************************************************************************
 *
 *  $RCSfile: RemoteServiceTest.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:47:54 $
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
