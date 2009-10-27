/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Relay.java,v $
 * $Revision: 1.6 $
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

package test.javauno.nativethreadpool;

import com.sun.star.bridge.BridgeExistsException;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.connection.AlreadyAcceptingException;
import com.sun.star.connection.ConnectionSetupException;
import com.sun.star.connection.Acceptor;
import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.lang.WrappedTargetRuntimeException;
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
        final XAcceptor acceptor = Acceptor.create(context);
        final XBridgeFactory factory;
        try {
            factory = UnoRuntime.queryInterface(
                XBridgeFactory.class,
                context.getServiceManager().createInstanceWithContext(
                    "com.sun.star.bridge.BridgeFactory", context));
        } catch (com.sun.star.uno.Exception e) {
            throw new WrappedTargetRuntimeException(e.toString(), this, e);
        }
        new Thread() {
            public void run() {
                try {
                    // Use "127.0.0.1" instead of "localhost", see #i32281#:
                    factory.createBridge(
                        "", "urp",
                        acceptor.accept("socket,host=127.0.0.1,port=3831"),
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
