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

package ifc.bridge;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Tests <code>com.sun.star.bridge.XBridgeFactory</code>
* interface methods :
* <ul>
*  <li><code> createBridge()</code></li>
*  <li><code> getBridge()</code></li>
*  <li><code> getExistingBridges()</code></li>
* </ul> <p>
* @see com.sun.star.bridge.XBridgeFactory
*/
public class _XBridgeFactory extends MultiMethodTest {

    public XBridgeFactory oObj = null;

    private String bridgeName = null ;

    AcceptorThread acceptorThread = null;

    /**
     * Interrupts the acceptor after test is finished
     */
    protected void after() {
        acceptorThread.acc.stopAccepting();
        if (acceptorThread.isAlive()) {
            acceptorThread.interrupt();
        }
    }
    /**
    * Calls <code>accept()</code> method in a separate thread.
    * Then stores exception thrown by call if it occurred, or
    * return value.
    */
    protected class AcceptorThread extends Thread {
        /**
         * the acceptor
         */
        private XAcceptor acc = null ;
        /**
        * If exception occurred during method call it is
        * stored in this field.
        */
        public Exception ex = null ;
        /**
        * If method call returns some value it stores in this field.
        */
        public XConnection acceptedCall = null ;

        /**
        * Gets an object which can call <code>accept</code> method.
        */
        public AcceptorThread(XAcceptor acc) {
            this.acc = acc ;
        }

        /**
        * Call <code>accept()</code> method.
        */
        public void run() {
            try {
                acceptedCall = acc.accept(connectString);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                ex = e ;
            } catch (com.sun.star.connection.ConnectionSetupException e) {
                ex = e ;
            } catch (com.sun.star.connection.AlreadyAcceptingException e) {
                ex = e ;
            }
        }
    }

    /**
    * Variable to make bridge names unique in different threads.
    */
    public static int uniqueSuffix = 0 ;
    /**
    * Object for synchronizing <code>uniqueSuffix</code> increment.
    */
    public static Object synchFlag = new Object() ;
    /**
     * Connection string
     */
     public String connectString;

    /**
    * Gets array of existing bridges. <p>
    * Has <b>OK</b> status if method returns not null.
    */
    public void _getExistingBridges() {

        XBridge[] bridges = oObj.getExistingBridges() ;

        log.println("Existing bridges :") ;
        for (int i = 0; i < bridges.length; i++)
            log.println("  " + bridges[i].getDescription()) ;

        if (bridges.length > 0) bridgeName = bridges[0].getName() ;

        tRes.tested("getExistingBridges()", bridges != null) ;
    }

    /**
    * First creates connection with StarOffice process, using environment
    * property <code>'CNCSTR'</code>. Then cerates bridge with unique name
    * using protocol specified in environment as <code>'PROTOCOL'</code>
    * property. After that bridge is disposed. <p>
    * Has <b>OK</b> status if value returned is not null
    * and no exceptions were thrown.<p>
    */
    public void _createBridge() {
        XBridge bridge = null;
        XConnection conn = null ;
        boolean result = false ;

        // first creating a connection
        try {
            XInterface x = (XInterface)
                ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.connection.Connector") ;

            XConnector xCntr = (XConnector) UnoRuntime.queryInterface
                (XConnector.class, x) ;

            x = (XInterface) ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.connection.Acceptor") ;

            XAcceptor xAccptr = (XAcceptor)UnoRuntime.queryInterface(
                                                        XAcceptor.class, x);
            connectString = (String)tEnv.getObjRelation("CNNCTSTR");
            acceptorThread = new AcceptorThread(xAccptr) ;
            acceptorThread.start();

            try {
                Thread.sleep(500);
            }
            catch (java.lang.InterruptedException e) {}
            conn = xCntr.connect(connectString) ;

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Can't create connection", e);
        }

        try {
            String protocol = (String) tParam.get("PROTOCOL") ;
            if (protocol == null) protocol = "urp" ;

            String brName ;
            synchronized (synchFlag) {
                brName = "MyBridge" + (uniqueSuffix++) ;
            }

            log.println("Creating bridge with name " + brName) ;

            bridge = oObj.createBridge(brName,
                protocol, conn, null) ;


            result = bridge != null ;
        } catch (com.sun.star.bridge.BridgeExistsException e) {
            log.println("Exception while bridge creating :" + e) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while bridge creating :" + e) ;
        }

        tRes.tested("createBridge()", result) ;
    }

    /**
    * Gets bridge by name and checks the bridge name returned. <p>
    * The following method tests are to be executed before :
    * <ul>
    * <li> <code>getExestingBridges</code> : to have some bridge name
    *    to retrieve </li>
    * </ul> <p>
    * Has <b>OK</b> status if bridge successfully returned and it's name
    * equals to name passed as parameter.
    */
    public void _getBridge() {
        executeMethod("getExistingBridges()") ;

        if (bridgeName == null) {
            log.println("No name for getting the bridge") ;
            return ;
        }

        XBridge br = oObj.getBridge(bridgeName) ;

        tRes.tested("getBridge()", br != null &&
            bridgeName.equals(br.getName())) ;
    }
}

