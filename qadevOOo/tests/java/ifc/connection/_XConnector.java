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

package ifc.connection;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.lang.XMultiServiceFactory;
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
            x = (XInterface) (
                (XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.connection.Acceptor") ;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Can't create service", e) ;
        }

        xAcceptor = (XAcceptor)UnoRuntime.queryInterface(XAcceptor.class, x);

        acceptorThread = new AcceptorThread(xAcceptor) ;
        acceptorThread.start() ;

        try {
            Thread.sleep(500);
        }
        catch (java.lang.InterruptedException e) {}

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

