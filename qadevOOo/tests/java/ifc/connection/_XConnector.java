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

package ifc.connection;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Tests methods of <code>XConnector</code> interface. <p>
* Required relations :
* <ul>
* <li> <code>'XConnector.connectStr'</code> : String variable. Has
*   the following format :
*   <code>'socket,host=<SOHost>,port=<UniquePort>' where <SOHost> is
*   the host where StarOffice is started. This string must be passed
*   as parameter to <code>accept()</code> method. </li>
* <ul> <p>
* This test <b>can not</b> be run in multiply threads.
*/
public class _XConnector extends MultiMethodTest {

    /**
    * Calls <code>accept()</code> method in a separate thread.
    * Then stores exception thrown by call if it occurred, or
    * return value.
    */
    protected class AcceptorThread extends Thread {
        /**
         * the acceptor
         */
        private final XAcceptor acc;
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
        @Override
        public void run() {
            try {
                acceptedCall = acc.accept(connectString) ;
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                ex = e ;
            } catch (com.sun.star.connection.ConnectionSetupException e) {
                ex = e ;
            } catch (com.sun.star.connection.AlreadyAcceptingException e) {
                ex = e ;
            }
        }
    }

    public XConnector oObj = null;
    protected String connectString = null ;

    /**
    * Retrieves object relation.
    */
    @Override
    public void before() throws StatusException {
        connectString = (String)
            tEnv.getObjRelation("XConnector.connectStr") ;
        if (connectString == null)
            throw new StatusException("No object relation found",
                new NullPointerException()) ;
    }

    /**
    * Thread with acceptor is created, and it starts listening.
    * The main thread tries to connect to acceptor. Acception thread must
    * return and a valid connection must be returned by Acceptor. <p>
    *
    */
    public void _connect() {
        boolean result = true ;
        AcceptorThread acceptorThread = null;
        XAcceptor xAcceptor = null ;
        XConnection aCon = null;
        XInterface x = null;

        // create the acceptor
        try {
            x = (XInterface) tParam.getMSF().createInstance
                ("com.sun.star.connection.Acceptor") ;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Can't create service", e) ;
        }

        xAcceptor = UnoRuntime.queryInterface(XAcceptor.class, x);

        acceptorThread = new AcceptorThread(xAcceptor) ;
        acceptorThread.start() ;

        util.utils.shortWait();

        // connect to acceptor
        try {
            aCon = oObj.connect(connectString);

            if (aCon == null)
                log.println("Connector returned: null") ;
            else
                log.println("Connector returned: " + aCon.getDescription()) ;

            try {
                acceptorThread.join(30 * 1000) ;
            } catch(InterruptedException e) {}

            // connection not established
            if (acceptorThread.isAlive()) {

                result = false ;
                log.println("Method call hasn't returned") ;

                if (acceptorThread.acceptedCall == null)
                    log.println("Acceptor returned : null") ;
                else
                    log.println("Acceptor returned : " +
                        acceptorThread.acceptedCall.getDescription()) ;
            } else {
                if (acceptorThread.ex != null) {
                    log.println("Exception occurred in accept() thread :") ;
                    acceptorThread.ex.printStackTrace(log) ;
                }

                if (acceptorThread.acceptedCall == null)
                    log.println("Method returned : null") ;
                else
                    log.println("Method returned : " +
                        acceptorThread.acceptedCall.getDescription()) ;

                result &= acceptorThread.acceptedCall != null ;
            }
        } catch (com.sun.star.connection.ConnectionSetupException e) {
            e.printStackTrace(log) ;
            result =  false ;
        } catch (com.sun.star.connection.NoConnectException e) {
            e.printStackTrace(log) ;
            result =  false ;
        } finally {
            acceptorThread.acc.stopAccepting();
            if (acceptorThread.isAlive()) {
                acceptorThread.interrupt();
            }
        }

        tRes.tested("connect()", result) ;
    }
}

