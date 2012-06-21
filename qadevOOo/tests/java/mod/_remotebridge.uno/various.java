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

package mod._remotebridge.uno;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;


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
* threads concurently.
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
    private int curPort = 50000;

    private XAcceptor xAcctr;
    private XConnector xCntr;
    private XBridgeFactory xBrdgFctr;
    private AcceptorThread accThread;

    public XInterface bridge = null;

    /**
     * Implementation of interface XInstanceProvider
     *
     * @see com.sun.star.bridge.XInstanceProvider
     */
    protected class MyInstanceProvider implements XInstanceProvider {
        /**
         * a MultiServiceFactory for creating instances
         *
         * @see com.sun.star.lang.MultiServiceFactory
         */
        private XMultiServiceFactory xMSF = null;

        /**
         * Construct object with a MultiServiceFactory
         *
         * @see com.sun.star.lang.MultiServiceFactory
         */
        public MyInstanceProvider(XMultiServiceFactory xMSF) {
            this.xMSF = xMSF;
        }

        /**
         * get an instance by name
         */
        public Object getInstance(String aInstanceName)
                        throws com.sun.star.container.NoSuchElementException
                        {
            System.out.println("######## Try to get "+aInstanceName);
            try {
                return xMSF.createInstance(aInstanceName);
            }
            catch(com.sun.star.uno.Exception e) {
                throw new StatusException("Unexpected exception", e);
            }
        }
    }

    /**
    * Calls <code>accept()</code> method in a separate thread.
    * Then stores exception thrown by call if it occurred, or
    * return value.
    */
    protected class AcceptorThread extends Thread {
        /**
        * If exception occurred during method call it is
        * stored in this field.
        */
        public Exception ex = null ;
        private XAcceptor acc = null ;
        private XInstanceProvider xInstProv = null ;
        private XBridgeFactory xBrdgFctr = null;
        /**
        * If method call returns some value it stores in this field.
        */
        public XConnection acceptedCall = null ;

        /**
        * Creates object which can call <code>accept</code> method
        * of the Acceptor object specified.
        */
        public AcceptorThread(XAcceptor acc, XInstanceProvider xInstProv,
                XBridgeFactory xBrdgFctr) {
            this.acc = acc ;
            this.xInstProv = xInstProv;
            this.xBrdgFctr = xBrdgFctr;
        }

        /**
        * Call <code>accept()</code> method and establish a bridge with an
        * instance provider
        */
        public void run() {
            try {
                acceptedCall = acc.accept(connectString) ;
                xBrdgFctr.createBridge("MyBridge", "urp",
                                            acceptedCall, xInstProv);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                ex = e ;
            } catch (com.sun.star.connection.ConnectionSetupException e) {
                ex = e ;
            } catch (com.sun.star.connection.AlreadyAcceptingException e) {
                ex = e ;
            } catch (com.sun.star.bridge.BridgeExistsException e) {
                ex = e ;
            }
        }
    }

    private final boolean[] bridgeDisposed = new boolean[1] ;

    /**
    * Creating a Testenvironment for the interfaces to be tested.
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
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
            PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        try {
            XInterface xInt = (XInterface)xMSF.createInstance(
                    "com.sun.star.bridge.Bridge");

            TestEnvironment tEnv = new TestEnvironment(xInt);
            // creating arguments for XInitialization
            // first, creating a connection
            // connection string
            String cncstr = (String) tParam.get("CNCSTR") ;
            int idx = cncstr.indexOf("host=") + 5 ;

            // select the port
//            curPort; //utils.getNextFreePort(basePort);
            log.println("Choose Port nr: " + curPort);

            connectString = "socket,host=" +
                    cncstr.substring(idx, cncstr.indexOf(",", idx)) +
                    ",port=" + curPort;

            // create acceptor
            XInterface oAcctr = (XInterface)xMSF.createInstance(
                    "com.sun.star.connection.Acceptor") ;

            xAcctr = (XAcceptor)UnoRuntime.queryInterface(
                    XAcceptor.class, oAcctr);
            // create connector
            XInterface oCntr = (XInterface)xMSF.createInstance(
                    "com.sun.star.connection.Connector") ;
            xCntr = (XConnector)UnoRuntime.queryInterface(
                    XConnector.class, oCntr);

            // create bridge factory
            XInterface oBrdg = (XInterface)xMSF.createInstance(
                    "com.sun.star.bridge.BridgeFactory") ;
            xBrdgFctr = (XBridgeFactory)
                        UnoRuntime.queryInterface(XBridgeFactory.class, oBrdg);

            // create own implementation of XInstanceProvider
            XInstanceProvider xInstProv = new MyInstanceProvider(xMSF);
            // create waiting acceptor thread
            accThread = new AcceptorThread(xAcctr, xInstProv, xBrdgFctr);
            accThread.start();
            // let the thread sleep
            try {
                Thread.sleep(500);
            }
            catch (java.lang.InterruptedException e) {}

            // establish the connection
            XConnection xConnection = xCntr.connect(connectString);

            String protocol = "urp";
            String bridgeName = protocol + ":" + connectString;

/*            bridgeDisposed[0] = false ;
            XComponent xComp = (XComponent)UnoRuntime.queryInterface(
                XComponent.class, xInt);
            final PrintWriter logF = log;
            xComp.addEventListener(new XEventListener() {
                public void disposing(EventObject ev) {
                    bridgeDisposed[0] = true ;
                    logF.println("The bridge Disposed.");
                }
            });
*/
            tEnv.addObjRelation("XInitialization.args", new Object[] {
                    bridgeName, protocol, xConnection, null});

            bridge = tEnv.getTestObject();

            return tEnv;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }
    }

    /**
     * Stop the acceptor thread and dispose the bridge
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {

        System.out.println("++++++++ cleanup");
        xAcctr.stopAccepting();
        if (accThread.isAlive()) {
            accThread.interrupt();
        }
        XComponent xComp = (XComponent)UnoRuntime.queryInterface(
                XComponent.class, xAcctr);
        if (xComp != null)
            xComp.dispose();
        xComp = (XComponent)UnoRuntime.queryInterface(
                XComponent.class, xCntr);
        if (xComp != null)
            xComp.dispose();
        xComp = (XComponent)UnoRuntime.queryInterface(
                XComponent.class, xBrdgFctr);
        if (xComp != null)
            xComp.dispose();

        xComp = (XComponent)UnoRuntime.queryInterface(
                XComponent.class, bridge);
        if (xComp != null) {
            System.out.println("######## Dispose bridge");
            bridgeDisposed[0] = true;
            xComp.dispose();
            // wait for dispose
            try {
                Thread.sleep(5000);
            }
            catch(java.lang.InterruptedException e) {
            }
        }
    }
}
