/*************************************************************************
 *
 *  $RCSfile: Relay.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-09 10:20:18 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package test.javauno.nativethreadpool;

import com.sun.star.bridge.BridgeExistsException;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.connection.AlreadyAcceptingException;
import com.sun.star.connection.ConnectionSetupException;
import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.lang.WrappedTargetRuntimeException;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public final class Relay implements XRelay, XSource {
    public void start(XSource source) {
        this.source = source;
        XComponentContext context;
        try {
            context = Bootstrap.createInitialComponentContext(null);
        } catch (RuntimeException e) {
            throw e;
        } catch (com.sun.star.uno.Exception e) {
            throw new WrappedTargetRuntimeException(e.toString(), this, e);
        } catch (Exception e) {
            throw new com.sun.star.uno.RuntimeException(e.toString(), this);
        }
        XMultiComponentFactory factory = context.getServiceManager();
        final XBridgeFactory bridgeFactory;
        final XAcceptor acceptor;
        try {
            bridgeFactory = (XBridgeFactory) UnoRuntime.queryInterface(
                XBridgeFactory.class,
                factory.createInstanceWithContext(
                    "com.sun.star.bridge.BridgeFactory", context));
            acceptor = (XAcceptor) UnoRuntime.queryInterface(
                XAcceptor.class,
                factory.createInstanceWithContext(
                    "com.sun.star.connection.Acceptor", context));
        } catch (RuntimeException e) {
            throw e;
        } catch (com.sun.star.uno.Exception e) {
            throw new WrappedTargetRuntimeException(e.toString(), this, e);
        }
        new Thread() {
            public void run() {
                try {
                    bridgeFactory.createBridge(
                        "", "urp",
                        acceptor.accept("socket,host=localhost,port=3831"),
                        new XInstanceProvider() {
                            public Object getInstance(String instanceName) {
                                return Relay.this;
                            }
                        });
                } catch (AlreadyAcceptingException e) {
                    e.printStackTrace(System.err);
                } catch (ConnectionSetupException e) {
                    e.printStackTrace(System.err);
                } catch (BridgeExistsException e) {
                    e.printStackTrace(System.err);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    e.printStackTrace(System.err);
                }
            }
        }.start();
        try {
            Thread.sleep(3000); // wait for new thread to accept connection
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new com.sun.star.uno.RuntimeException(e.toString(), this);
        }
    }

    public int get() {
        return source.get();
    }

    public static XSingleServiceFactory __getServiceFactory(
        String implName, XMultiServiceFactory multiFactory, XRegistryKey regKey)
    {
        return implName.equals(implementationName)
            ? FactoryHelper.getServiceFactory(
                Relay.class, serviceName, multiFactory, regKey)
            : null;
    }

    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(
            implementationName, serviceName, regKey);
    }

    private static final String implementationName
    = "test.javauno.nativethreadpool.comp.Relay";
    private static final String serviceName
    = "test.javauno.nativethreadpool.Relay";

    private XSource source;
}
