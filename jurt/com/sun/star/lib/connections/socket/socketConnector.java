/*************************************************************************
 *
 *  $RCSfile: socketConnector.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:27:52 $
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

package com.sun.star.lib.connections.socket;


import java.io.IOException;

import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;


import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.registry.XRegistryKey;

/**
 * The socketConnector class is a component,
 * that implements the <code>XConnector</code> Interface.
 * <p>
 * The socketConnector is a specialized component, which uses
 * <code>sockets</code> for communication.
 * The socketConnector is in general used by the Connector service.
 * <p>
 * @version     $Revision: 1.1.1.1 $ $ $Date: 2000-09-18 15:27:52 $
 * @author      Kay Ramme
 * @see         com.sun.star.connections.XAcceptor
 * @see         com.sun.star.connections.XConnector
 * @see         com.sun.star.connections.XConnection
 * @see         com.sun.star.loader.JavaLoader
 * @since       UDK1.0
 */
public class socketConnector implements XConnector {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    /**
     * The name of the service, the <code>JavaLoader</code> acceses this through reflection.
     */
    static public final String __serviceName = "com.sun.star.connection.socketConnector";

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

        if (implName.equals(socketConnector.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory(socketConnector.class,
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
        return FactoryHelper.writeRegistryServiceInfo(socketConnector.class.getName(), __serviceName, regKey);
    }




    protected String _description;
    protected String _hostname     = "0";
    protected int    _port         = 6001;

    /**
     * Connect through the described mechanism to a waiting server.
     * <p>
     * The description has the following format:
     * [,attribute_name=attribute_value]
     * Supported attributes at the moment are:
     * <dl>
     * <li> host - the name of interface (defaults to 0 == all interfaces
     * <li> port - the port number (default to 6001)
     * </dl>
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
        if(_description != null)
            throw new com.sun.star.connection.ConnectionSetupException(getClass().getName() + ".connect - alread connected");

        _description = description.trim();

        // find hostname
        int index = _description.indexOf(':');
        if(index >= 0) { // old style
            _hostname = _description.substring(0, index);
            _description = _description.substring(index + 1);

            // find port
            index = _description.indexOf(':');
            if(index > -1) {
                _port = Integer.parseInt(_description.substring(0, index));
                _description = _description.substring(index + 1);
            }
            else
                _port = Integer.parseInt(_description);
        }
        else { // new style: is comma separated list
            while(_description.length() > 0) {
                index = _description.indexOf(',');

                String word = null;

                if(index >= 0) {
                    word = _description.substring(0, index).trim();
                    _description = _description.substring(index + 1).trim();
                }
                else {
                    word = _description.trim();
                    _description = "";
                }

                index = word.indexOf('=');
                String left = word.substring(0, index).trim();
                String right = word.substring(index + 1).trim();

                if(left.equals("host"))
                    _hostname = right;
                else if(left.equals("port"))
                    _port = Integer.parseInt(right);
                else
                    System.err.println(getClass().getName() + ".connect - unknown attribute:" + left);
            }
        }

        XConnection xConnection = null;

        try {
            InetAddress inetAddress = InetAddress.getByName(_hostname);
            Socket socket = new Socket(inetAddress, _port);

            xConnection = new SocketConnection(description + socket, socket);
        }
        catch(UnknownHostException unknownHostException) {
            throw new com.sun.star.connection.ConnectionSetupException(unknownHostException.toString());
        }
        catch(IOException ioException) {
            throw new com.sun.star.connection.NoConnectException(ioException.toString());
        }

        return xConnection;
    }
}

