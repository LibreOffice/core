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

package test.java_uno.nativethreadpool;

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
