/*************************************************************************
 *
 *  $RCSfile: Connector.java,v $
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


import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.registry.XRegistryKey;

import com.sun.star.uno.UnoRuntime;

/**
 * The Connector class is a component that implements the <code>XConnector</code> Interface.
 * <p>
 * The Connector is a general component, which uses less
 * general components (like <code>SocketConnector</code>) to
 * implement its functionality.
 * <p>
 * @version     $Revision: 1.1.1.1 $ $ $Date: 2000-09-18 15:27:51 $
 * @author      Kay Ramme
 * @see         com.sun.star.connections.XAcceptor
 * @see         com.sun.star.connections.XConnector
 * @see         com.sun.star.connections.XConnection
 * @see         com.sun.star.loader.JavaLoader
 * @since       UDK1.0
 */
public class Connector implements XConnector {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    /**
     * The name of the service, the <code>JavaLoader</code> acceses this through reflection.
     */
     static private final String __serviceName = "com.sun.star.connection.Connector";

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

        if (implName.equals(Connector.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory(Connector.class,
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
        return FactoryHelper.writeRegistryServiceInfo(Connector.class.getName(), __serviceName, regKey);
    }


    protected XMultiServiceFactory _xMultiServiceFactory;
    protected String               _description;
    protected String               _mech;
    protected XConnector           _connector;


    /**
     * Constructs a new <code>SocketConnector</code>,
     * which uses the given <code>XMultiServiceFactory</code>
     * as its service manager.
     * <p>
     * @param xMultiServiceFactory  the service manager
     */
    public Connector(XMultiServiceFactory xMultiServiceFactory) {
        _xMultiServiceFactory = xMultiServiceFactory;
    }

    /**
     * Connect through the described mechanism to a waiting server.
     * <p>
     * The description has the following format:
     * &lt;interface_type&gt;[,attribute_name=attribute_value]
     * The interface type is searched for in <code>com.sun.star.lib.connections</code>
     * <p>
     * @return  an <code>XConnection</code> to the client
     * @param   description    the description of the network interface
     * @see     com.sun.star.connections.XAcceptor
     * @see     com.sun.star.connections.XConnection
     */
    public synchronized XConnection connect(String description)
        throws com.sun.star.connection.NoConnectException,
               com.sun.star.connection.ConnectionSetupException,
               com.sun.star.uno.RuntimeException
    {
        if (DEBUG) System.err.println("##### " + getClass().getName() + ".connect:" + description);

        if(_description != null)
            throw new com.sun.star.connection.ConnectionSetupException(getClass().getName() + ".connect - alread connected");

        _description = description.trim();

        // find mechanism - try old style (e.g.: socket:localhost:6001) first
        int index = _description.indexOf(':');
        if(index >= 0) {
            _mech = _description.substring(0, index);
            _description = _description.substring(index + 1).trim();
        }
        else { // must be new style (e.g.: socket, name=sdfas, ...)
            index = _description.indexOf(',');
            if(index >= 0) { // there are parameters
                _mech = _description.substring(0, index);
                _description = _description.substring(index + 1).trim();
            }
            else {
                _mech = description;
                _description = "";
            }
        }
        _mech = _mech.trim().toLowerCase();

        try {
            // try to get a service to which we can delegate
              _connector = (XConnector)UnoRuntime.queryInterface(XConnector.class, _xMultiServiceFactory.createInstance("com.sun.star.connection.Connector." + _mech));
        }
          catch(com.sun.star.uno.Exception exception) {
            // if this does not work, we a fall back (this is also old style)

            try { // try to find the class in out package structure
                Class connectorClass = Class.forName("com.sun.star.lib.connections." + _mech + "." + _mech + "Connector");
                _connector = (XConnector)connectorClass.newInstance();
            }
            catch(Exception e) {
            }
        }

        if(_connector == null)
            throw new com.sun.star.connection.ConnectionSetupException(getClass().getName() + ".connect - can not find delegatee:" + _mech);

        return _connector.connect(_description);
    }
}

