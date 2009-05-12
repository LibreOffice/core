/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Connector.java,v $
 * $Revision: 1.3 $
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
package basicrunner.basichelper;

import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.ConnectionSetupException;
import com.sun.star.connection.NoConnectException;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XSingleServiceFactory;

/**
 * This is a special service that is used in testing Acceptor
 * component in BASIC. This componennt creates a separate thread
 * that tries to connect to BASIC's acceptor. After successfull
 * connection it writes a connectionString to XConnection.
 */
 public class Connector implements XServiceInfo, XSingleServiceFactory {
    /** The service name of this class **/
    static final String __serviceName = "basichelper.Connector";
    /** The Connector implementation **/
    static ConnectorImpl oConnector = null;

   /** Create a connector.
    */
   public Connector() {
        oConnector = new ConnectorImpl();
    }

    /**
     * Returns an instance of the connector.
     * Arguments are not supported here and will be ignored.
     * @param args The arguments.
     * @return The connector.
     */
    public Object createInstanceWithArguments(Object[] args) {
        return oConnector;
    }

    /**
     * Returns an instance of the connector.
     * @return The connector.
     */
    public Object createInstance() {
        return createInstanceWithArguments(null);
    }

    /**
     * Get a unique id for this implementation.
     * @return The id.
     */
    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    /**
     * Return all implemented types of this class.
     * @return The implemented UNO types.
     */
    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();

        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);

        return types;
    }

    /** Is this servioce supported?
     * @param name The service name.
     * @return True, if the service is supported.
     */
    public boolean supportsService(String name) {
        return __serviceName.equals(name);
    }

    /**
     * Get all supported service names.
     * @return All supported servcices.
     */
    public String[] getSupportedServiceNames() {
        return new String[] {__serviceName};
    }

    /**
     * Get the implementation name of this class.
     * @return The implementation name.
     */
    public String getImplementationName() {
        return getClass().getName();
    }
}

/**
 * The actual implementation of the connector
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.lang.XTypeProvider
 * @see com.sun.star.container.XNameAccess
 */
class ConnectorImpl implements XInitialization, XTypeProvider, XNameAccess {
    static String aState;
    static Integer iTimeout;

    /**
     * Construct a new connector.
     */
    public ConnectorImpl() {
        aState = "just created";
        iTimeout = new Integer(3000);
    }

    /**
     * Method initialize() creates a new thread that will try to connect to
     * Acceptor for a few seconds. One should pass as parameters an array,
     * where element 0 is an instance of Connector and element 1 is a
     * connection string (the same as in Acceptor)
     * @param parm1 An instance of XConnector.
     * @see com.sun.star.connection.XConnector
     * @throws Exception Is thrown, when initialize fails.
     */
    public void initialize(Object[] parm1) throws com.sun.star.uno.Exception {
        aState = "just initialized";
        XConnector cntr = (XConnector)UnoRuntime.queryInterface(
                                                XConnector.class, parm1[0]);
        ConnThread aThread = new ConnThread(cntr, (String)parm1[1]);
        aThread.start();
    }

    /**
     * Get the element names
     * @return All element names.
     */
    public String[] getElementNames() {
        return new String[]{"State", "Timeout"};
    }

    /**
     * Does this element exist?
     * @param name The element name.
     * @return True, if the name exists.
     */
    public boolean hasByName(String name) {
        return (name.equals("State") || name.equals("Timeout"));
    }

    /**
     * Get an element by its name.
     * @param name The name of the element.
     * @return The value of the element.
     * @throws NoSuchElementException The element does not exist.
     */
    public Object getByName(String name) throws NoSuchElementException{
        if (name.equals("State"))
            return aState;
        else if (name.equals("Timeout"))
            return iTimeout;
        else
            throw new NoSuchElementException();
    }

    /**
     * Are there elements
     * @return Always true.
     */
    public boolean hasElements() {
        return true;
    }

    /**
     * Get element type.
     * @return null.
     */
    public Type getElementType() {
        return null;
    }

    /**
     * Get a unique id for this implementation.
     * @return The id.
     */
    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    /**
     * Return all implemented types of this class.
     * @return The implemented UNO types.
     */
    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();

        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);

        return types;
    }
}

/**
 * A connector thread
 */
class ConnThread extends Thread {
    String connStr;
    XConnector oConnector;

    /**Construct the thread.
     * @param oCntr A connector.
     * @param cStr The conection string.
     */
    public ConnThread(XConnector oCntr, String cStr){
        connStr = cStr;
        oConnector = oCntr;
    }

    /**
     * Run the thread.
     */
    public void run(){
        try {
            Thread.sleep(ConnectorImpl.iTimeout.intValue());
            ConnectorImpl.aState = "before connection";
            XConnection oConnection = oConnector.connect(connStr);
            if (oConnection != null) {
                ConnectorImpl.aState = "connected";
                oConnection.write(connStr.getBytes());
                oConnection.write(new byte[]{0});
            } else
                ConnectorImpl.aState = "XConnection is null";
        } catch (ConnectionSetupException e) {
            ConnectorImpl.aState = "ConnectionSetupException";
            throw new RuntimeException(e.toString());
        } catch (NoConnectException e) {
            ConnectorImpl.aState = "NoConnectException";
            throw new RuntimeException(e.toString());
        } catch (Exception e) {
            ConnectorImpl.aState = "error";
            throw new RuntimeException("Can't sleep exception");
        }
    }
}
