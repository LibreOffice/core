/*************************************************************************
 *
 *  $RCSfile: ConstantInstanceProvider.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:27:51 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.comp.connections;

import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.registry.XRegistryKey;

import com.sun.star.comp.loader.FactoryHelper;


/**
 * The <code>ConstantInstanceProvider</code> is a component
 * that implements the <code>XInstanceProvider</code> Interface.
 * <p>
 * @version     $Revision: 1.1.1.1 $ $ $Date: 2000-09-18 15:27:51 $
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

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(ConstantInstanceProvider.class.getName(), __serviceName, regKey);
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

