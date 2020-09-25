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

package mod._remotebridge;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.bridge.Bridge</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XInitialization</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::bridge::XBridge</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurrently.
* @see com.sun.star.lang.XInitialization
* @see com.sun.star.lang.XComponent
* @see com.sun.star.bridge.XBridge
* @see com.sun.star.bridge.Bridge
* @see ifc.lang._XInitialization
* @see ifc.lang._XComponent
* @see ifc.bridge._XBridge
*/
public class various extends TestCase {

    /**
     *  String for establishing a connection
     */
    protected String connectString = null ;

    /**
    * Choose the first port after <code>basePort</code>
    * which is free.
    */
    protected static final int basePort = 50000;
    private static final int curPort = 50000;

    private XAcceptor xAcctr;
    private XConnector xCntr;
    private XBridgeFactory xBrdgFctr;
    private AcceptorThread accThread;

    public XInterface bridge = null;

    /**
    * Calls <code>accept()</code> method in a separate thread.
    * Then stores exception thrown by call if it occurred, or
    * return value.
    */
    private class AcceptorThread extends Thread {
        private final XAcceptor acc;

        /**
        * Creates object which can call <code>accept</code> method
        * of the Acceptor object specified.
        */
        private AcceptorThread(XAcceptor acc) {
            this.acc = acc ;
        }

        /**
        * Call <code>accept()</code> method and establish a bridge with an
        * instance provider
        */
        @Override
        public void run() {
            try {
                acc.accept(connectString) ;
            } catch (com.sun.star.connection.ConnectionSetupException e) {
            } catch (com.sun.star.connection.AlreadyAcceptingException e) {
            }
        }
    }

    private final boolean[] bridgeDisposed = new boolean[1] ;

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.bridge.Bridge</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XInitialization.args'</code> for
    *   {@link ifc.lang._XInitialization} and
    *   {@link ifc.bridge._XBridge} : contains arguments
    *   for <code>initialize()</code> method test.</li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
            PrintWriter log) throws Exception {
        XMultiServiceFactory xMSF = tParam.getMSF();

        XInterface xInt = (XInterface)xMSF.createInstance(
                "com.sun.star.bridge.Bridge");

        TestEnvironment tEnv = new TestEnvironment(xInt);
        // creating arguments for XInitialization
        // first, creating a connection
        // connection string
        String cncstr = (String) tParam.get("CONNECTION_STRING") ;
        int idx = cncstr.indexOf("host=") + 5 ;

        // select the port
        log.println("Choose Port nr: " + curPort);

        connectString = "socket,host=" +
                cncstr.substring(idx, cncstr.indexOf(",", idx)) +
                ",port=" + curPort;

        // create acceptor
        XInterface oAcctr = (XInterface)xMSF.createInstance(
                "com.sun.star.connection.Acceptor") ;

        xAcctr = UnoRuntime.queryInterface(
                XAcceptor.class, oAcctr);
        // create connector
        XInterface oCntr = (XInterface)xMSF.createInstance(
                "com.sun.star.connection.Connector") ;
        xCntr = UnoRuntime.queryInterface(
                XConnector.class, oCntr);

        // create bridge factory
        XInterface oBrdg = (XInterface)xMSF.createInstance(
                "com.sun.star.bridge.BridgeFactory") ;
        xBrdgFctr = UnoRuntime.queryInterface(XBridgeFactory.class, oBrdg);

        // create waiting acceptor thread
        accThread = new AcceptorThread(xAcctr);
        accThread.start();
        // let the thread sleep
        util.utils.pause(500);

        // establish the connection
        XConnection xConnection = xCntr.connect(connectString);

        String protocol = "urp";
        String bridgeName = protocol + ":" + connectString;

        tEnv.addObjRelation("XInitialization.args", new Object[] {
                bridgeName, protocol, xConnection, null});

        bridge = tEnv.getTestObject();

        return tEnv;
    }

    /**
     * Stop the acceptor thread and dispose the bridge
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {

        System.out.println("++++++++ cleanup");
        xAcctr.stopAccepting();
        if (accThread.isAlive()) {
            accThread.interrupt();
        }
        XComponent xComp = UnoRuntime.queryInterface(
                XComponent.class, xAcctr);
        if (xComp != null)
            xComp.dispose();
        xComp = UnoRuntime.queryInterface(
                XComponent.class, xCntr);
        if (xComp != null)
            xComp.dispose();
        xComp = UnoRuntime.queryInterface(
                XComponent.class, xBrdgFctr);
        if (xComp != null)
            xComp.dispose();

        xComp = UnoRuntime.queryInterface(
                XComponent.class, bridge);
        if (xComp != null) {
            System.out.println("######## Dispose bridge");
            bridgeDisposed[0] = true;
            xComp.dispose();
            // wait for dispose
            util.utils.pause(5000);
        }
    }
}
