/*************************************************************************
 *
 *  $RCSfile: Acceptor.java,v $
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


import java.io.IOException;

import java.net.ServerSocket;
import java.net.Socket;


import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.uno.UnoRuntime;

import com.sun.star.registry.XRegistryKey;

/**
 * The Acceptor class is a component,
 * that implements the <code>XAcceptor</code> Interface.
 * <p>
 * The Acceptor is a general component, which uses less
 * general components (like <code>SocketAcceptor</code>) to
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
public class Acceptor implements XAcceptor {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    /**
     * The name of the service, the <code>JavaLoader</code> acceses this through reflection.
     */
    static private final String __serviceName = "com.sun.star.connection.Acceptor";

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

        if (implName.equals(Acceptor.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory(Acceptor.class,
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
        return FactoryHelper.writeRegistryServiceInfo(Acceptor.class.getName(), __serviceName, regKey);
    }



    protected XMultiServiceFactory _xMultiServiceFactory;
    protected XAcceptor            _acceptor;
    protected String               _acceptor_description;
    protected String               _mech;

    /**
     * Constructs a new <code>Acceptor</code> which uses the
     * given service factory for creating the concrete listener.
     * <p>
     * @param  xMultiServiceFactory  the service manager to use
     */
    public Acceptor(XMultiServiceFactory xMultiServiceFactory) {
        _xMultiServiceFactory = xMultiServiceFactory;
    }

    /**
     * Accepts a connect request through the described mechanism.
     * This call blocks until a connection has been established.
     * <p>
     * The description has the following format:
     * &lt;interface_type&gt;[,attribute_name=attribute_value]
     * The interface type is instantiated through the service manager as
     * <code>com.sun.star.connections.&lt;interface_type&gt;Acceptor</code>
     * <p>
     * @return  an <code>XConnection</code> to the client
     * @param   description    the description of the network interface
     * @see     com.sun.star.connections.XConnector
     * @see     com.sun.star.connections.XConnection
     */
    public XConnection accept(String description)
        throws com.sun.star.connection.AlreadyAcceptingException,
               com.sun.star.connection.ConnectionSetupException,
               com.sun.star.lang.IllegalArgumentException,
               com.sun.star.uno.RuntimeException
    {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".accept - description:" + description);

        String mech = null;
        description = description.trim().toLowerCase();

        // find mechanism - try old style (e.g.: socket:localhost:6001) first
        int index = description.indexOf(':');
        if(index >= 0) {
            mech = description.substring(0, index).trim();
            description = description.substring(index + 1).trim();
        }
        else { // must be new style (e.g.: socket, name=sdfas, ...)
            index = description.indexOf(',');
            if(index >= 0) { // there are parameters
                mech = description.substring(0, index).trim();
                description = description.substring(index + 1).trim();
            }
            else {
                mech = description.trim();
                description = "";
            }
        }

        if(_acceptor == null) {// this is the first time accept is called
              try {
                // at first we try to find the delegatee via service name
                  _acceptor = (XAcceptor)UnoRuntime.queryInterface(XAcceptor.class, _xMultiServiceFactory.createInstance("com.sun.star.connection.Acceptor" + mech));
              }
              catch(com.sun.star.uno.Exception exception) {
                try {
                    // try to find the class in out package structure
                    Class acceptorClass = Class.forName("com.sun.star.lib.connections." + mech + "." + mech + "Acceptor");

                    _acceptor = (XAcceptor)acceptorClass.newInstance();
                }
                catch(Exception e) {
                }
            }

            if(_acceptor == null)
                throw new com.sun.star.connection.ConnectionSetupException(getClass().getName() + ".accept - can not find delegatee:" + mech);

            _mech = mech;
        }
        else // this is not the first time, so the mechanics must be the same
            if(!_mech.equals(mech)) throw new com.sun.star.connection.AlreadyAcceptingException(getClass().getName() + ".accept");


        return _acceptor.accept(description);
    }

    /**
     * Unblocks the <code>accept</code> call.
     * Delegates to the concretes mechanism.
     */
    public void stopAccepting() throws com.sun.star.uno.RuntimeException {
        _acceptor.stopAccepting();
    }
}

