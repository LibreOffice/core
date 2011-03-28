/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package com.sun.star.comp.bridgefactory;

import java.math.BigInteger;
import java.util.Vector;


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
 * @version     $Revision: 1.11 $ $ $Date: 2008-04-11 11:07:51 $
 * @author      Kay Ramme
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
        catch(Exception exception) {
            throw new com.sun.star.lang.IllegalArgumentException(exception.getMessage());
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
        Vector vector = new Vector();

        IBridge iBridges[] = UnoRuntime.getBridges();
        for(int i = 0; i < iBridges.length; ++ i) {
            XBridge xBridge = UnoRuntime.queryInterface(XBridge.class, iBridges[i]);

            if(xBridge != null)
                vector.addElement(xBridge);
        }

        XBridge xBridges[]= new XBridge[vector.size()];
        for(int i = 0; i < vector.size(); ++ i)
            xBridges[i] = (XBridge)vector.elementAt(i);

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

