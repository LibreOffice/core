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

package com.sun.star.comp.connections;

import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.registry.XRegistryKey;

import com.sun.star.comp.loader.FactoryHelper;


/**
 * The <code>ConstantInstanceProvider</code> is a component
 * that implements the <code>XInstanceProvider</code> Interface.
 * <p>
 * @version     $Revision: 1.3 $ $ $Date: 2008-04-11 11:08:55 $
 * @author      Kay Ramme
 * @see         com.sun.star.bridge.XBridge
 * @see         com.sun.star.bridge.XBridgeFactory
 * @see         com.sun.star.bridge.XInstanceProvider
 * @see         com.sun.star.loader.JavaLoader
 * @since       UDK1.0
 */
public class ConstantInstanceProvider implements XInstanceProvider {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    /**
     * The name of the service, the <code>JavaLoader</code> acceses this through reflection.
     */
    static private final String __serviceName = "com.sun.star.comp.connection.InstanceProvider";

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

        if (implName.equals(ConstantInstanceProvider.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory(ConstantInstanceProvider.class,
                                                                    __serviceName,
                                                                    multiFactory,
                                                                    regKey);

        return xSingleServiceFactory;
    }

    protected XMultiServiceFactory _serviceManager;
    protected String _serviceName;
    protected Object _instance;


    public void setInstance(String serviceName) throws com.sun.star.uno.Exception {
        _instance = _serviceManager.createInstance(serviceName);
        _serviceName = serviceName;
    }

    /**
     * Constructs a new <code>ConstantInstanceProvider</code>.
     * Uses the provided ServiceManager as the provided instance.
     * <p>
     * @param    serviceName   the provided service manager
     */
    public ConstantInstanceProvider(XMultiServiceFactory serviceManager) {
        _serviceManager = serviceManager;

        _serviceName = "SERVICEMANAGER";
        _instance    = serviceManager;
    }

    /**
     * Gives an object for the passed instance name.
     * <p>
     * @return  the desired instance
     * @param   sInstanceName   the name of the desired instance
     */
    public Object getInstance(String sInstanceName) throws com.sun.star.container.NoSuchElementException, com.sun.star.uno.RuntimeException {
        Object result = sInstanceName.equals(_serviceName) ? _instance : null;

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".getInstance(" + sInstanceName + "):" + result);

        return result;
    }
}

