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

package com.sun.star.comp.bridgefactory;

import java.math.BigInteger;
import java.util.ArrayList;

import com.sun.star.bridge.BridgeExistsException;
import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.connection.XConnection;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.UnoRuntime;


/**
 * The BridgeFactory class implements the <code>XBridgeFactory</code> Interface.
 * It wrapps the <code>UnoRuntime#getBridgeByName</code>method and delivers a
 * XBridge component.
 * <p>
 * This component is only usable for remote bridges.
 * <p>
 * @see         com.sun.star.uno.UnoRuntime
 * @since       UDK1.0
 */
public class BridgeFactory implements XBridgeFactory/*, XEventListener*/ {
    static private final boolean DEBUG = false;

    /**
     * The name of the service, the <code>JavaLoader</code> acceses this through reflection.
     */
    public final static String __serviceName = "com.sun.star.bridge.BridgeFactory";

    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleServiceFactory</code> for creating the component
     * @param   implName     the name of the implementation for which a service is desired
     * @param   multiFactory the service manager to be uses if needed
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory(String implName,
                                                          XMultiServiceFactory multiFactory,
                                                          XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(BridgeFactory.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory(BridgeFactory.class,
                                                                    multiFactory,
                                                                    regKey);

        return xSingleServiceFactory;
    }

    /**
     * Creates a remote bridge and memorizes it under <code>sName</code>.
     * <p>
     * @return   the bridge
     * @param    sName                the name to memorize the bridge
     * @param    sProtocol            the protocol the bridge should use
     * @param    anInstanceProvider   the instance provider
     * @see                           com.sun.star.bridge.XBridgeFactory
     */
    public XBridge createBridge(String sName, String sProtocol, XConnection aConnection, XInstanceProvider anInstanceProvider) throws
        BridgeExistsException,
        com.sun.star.lang.IllegalArgumentException,
        com.sun.star.uno.RuntimeException
    {
        boolean hasName = sName.length() != 0;
        Object context = hasName ? sName : new UniqueToken();
            // UnoRuntime.getBridgeByName internally uses context.toString() to
            // distinguish bridges, so the result of
            // new UniqueToken().toString() might clash with an explicit
            // sName.toString(), but the UnoRuntime bridge management is
            // obsolete anyway and should be removed

        // do not create a new bridge, if one already exists
        if (hasName) {
            IBridge iBridges[] = UnoRuntime.getBridges();
            for(int i = 0; i < iBridges.length; ++ i) {
                XBridge xBridge = UnoRuntime.queryInterface(XBridge.class, iBridges[i]);

                if(xBridge != null) {
                    if(xBridge.getName().equals(sName))
                        throw new BridgeExistsException(sName + " already exists");
                }
            }
        }

        XBridge xBridge;

        try {
            IBridge iBridge = UnoRuntime.getBridgeByName("java", context, "remote", context, hasName ? new Object[]{sProtocol, aConnection, anInstanceProvider, sName} : new Object[]{sProtocol, aConnection, anInstanceProvider});

            xBridge = UnoRuntime.queryInterface(XBridge.class, iBridge);
        }
        catch (Exception e) {
            com.sun.star.lang.IllegalArgumentException e2 =
                new com.sun.star.lang.IllegalArgumentException(e.getMessage());
            e2.initCause(e);
            throw e2;
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".createBridge:" + sName + " " + sProtocol + " " + aConnection + " "  + anInstanceProvider + " " + xBridge);

        return xBridge;
    }

    /**
     * Gets a remote bridge which must already exist.
     * <p>
     * @return   the bridge
     * @param    sName                the name of the bridge
     * @see                           com.sun.star.bridge.XBridgeFactory
     */
    public XBridge getBridge(String sName) throws com.sun.star.uno.RuntimeException {
        XBridge xBridge = null;

        IBridge iBridges[] = UnoRuntime.getBridges();
        for(int i = 0; i < iBridges.length; ++ i) {
            xBridge = UnoRuntime.queryInterface(XBridge.class, iBridges[i]);

            if(xBridge != null) {
                if(xBridge.getName().equals(sName))
                    break;

                else
                    xBridge = null;
            }
        }


        if(DEBUG) System.err.println("##### " + getClass().getName() + ".getBridge:" + sName + " " + xBridge);

        return xBridge;
    }

    /**
     * Gives all created bridges
     * <p>
     * @return   the bridges
     * @see                           com.sun.star.bridge.XBridgeFactory
     */
    public synchronized XBridge[] getExistingBridges() throws com.sun.star.uno.RuntimeException {
        ArrayList<XBridge> vector = new ArrayList<XBridge>();

        IBridge iBridges[] = UnoRuntime.getBridges();
        for(int i = 0; i < iBridges.length; ++ i) {
            XBridge xBridge = UnoRuntime.queryInterface(XBridge.class, iBridges[i]);

            if(xBridge != null)
                vector.add(xBridge);
        }

        XBridge xBridges[]= new XBridge[vector.size()];
        for(int i = 0; i < vector.size(); ++ i)
            xBridges[i] = vector.get(i);

        return xBridges;
    }

    private static final class UniqueToken {
        public UniqueToken() {
            synchronized (UniqueToken.class) {
                token = counter.toString();
                counter = counter.add(BigInteger.ONE);
            }
        }

        public String toString() {
            return token;
        }

        private final String token;
        private static BigInteger counter = BigInteger.ZERO;
    }
}

