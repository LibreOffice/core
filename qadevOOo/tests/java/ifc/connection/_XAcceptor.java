/*************************************************************************
 *
 *  $RCSfile: _XAcceptor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:21:25 $
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

package ifc.connection;

import java.io.PrintWriter;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Tests methods of <code>XAcceptor</code> interface. <p>
* Required relations :
* <ul>
* <li> <code>'XAcceptor.connectStr'</code> : String variable. Has
*   the following format :
*   <code>'socket,host=<SOHost>,port=<UniquePort>' where <SOHost> is
*   the host where StarOffice is started. This string must be passed
*   as parameter to <code>accept()</code> method. </li>
* <ul> <p>
* This test <b>can not</b> be run in multiply threads.
*/
public class _XAcceptor extends MultiMethodTest {

    protected PrintWriter log_ ;

    /**
    * Calls <code>accept()</code> method in a separate thread.
    * Then stores exception thrown by call if it occured, or
    * return value.
    */
    protected class AcceptorThread extends Thread {
        /**
        * If exception occured during method call it is
        * stored in this field.
        */
        public Exception ex = null ;
        private XAcceptor acc = null ;
        /**
        * If method call returns some value it stores in this field.
        */
        public XConnection acceptedCall = null ;

        /**
        * Creates object which can call <code>accept</code> method
        * of the Acceptor object specified.
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

    public XAcceptor oObj = null;
    protected String connectString = null ;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
        connectString = (String)
            tEnv.getObjRelation("XAcceptor.connectStr") ;

        log_ = log ;

        if (connectString == null)
            throw new StatusException("No object relation found",
                new NullPointerException()) ;
    }

    /**
    * First part : Thread with acceptor created, and it starts listening.
    * The main thread tries to connect to acceptor. Acception thread must
    * return and valid connection must be returned by Acceptor. <p>
    *
    * Second part : Trying to create second acceptor which listen on
    * the same port. Calling <code>accept()</code> method of the second
    * Acceptor must rise appropriate exception. <p>
    *
    * Has OK status if both test parts executed properly.
    */
    public void _accept() {
        boolean result = true ;
        AcceptorThread acception = null,
                       dupAcception = null ;
        XAcceptor dupAcceptor = null ;
        XConnector xConnector = null ;

        // creating services requierd
        try {
            Object oConnector = ((XMultiServiceFactory)tParam.getMSF()).
                createInstance("com.sun.star.connection.Connector") ;

            xConnector = (XConnector) UnoRuntime.queryInterface
                (XConnector.class, oConnector) ;

            XInterface acceptor = (XInterface) ((XMultiServiceFactory)
                tParam.getMSF()).createInstance
                ("com.sun.star.connection.Acceptor") ;

            dupAcceptor = (XAcceptor) UnoRuntime.queryInterface
                (XAcceptor.class, acceptor) ;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Can't create service", e) ;
        }

        // Testing connection to the acceptor
        try {
            acception = new AcceptorThread(oObj) ;
            acception.start() ;

            try {
                Thread.sleep(500);
            }
            catch (java.lang.InterruptedException e) {}

            XConnection con = xConnector.connect(connectString) ;

            if (con == null)
                log.println("Connector returned : null") ;
            else
                log.println("Connector returned : " + con.getDescription()) ;

            try {
                acception.join(5 * 1000) ;
            } catch(InterruptedException e) {}

            if (acception.isAlive()) {

                result = false ;
                log.println("Method call haven't returned") ;

                if (acception.acceptedCall == null)
                    log.println("Acceptor returned : null") ;
                else
                    log.println("Acceptor returned : " +
                        acception.acceptedCall.getDescription()) ;
            } else {
                if (acception.ex != null) {
                    log.println("Exception occured in accept() thread :") ;
                    acception.ex.printStackTrace(log) ;
                }

                if (acception.acceptedCall == null)
                    log.println("Method returned : null") ;
                else
                    log.println("Method returned : " +
                        acception.acceptedCall.getDescription()) ;

                result &= acception.acceptedCall != null ;
            }
        } catch (com.sun.star.connection.ConnectionSetupException e) {
            e.printStackTrace(log) ;
            result =  false ;
        } catch (com.sun.star.connection.NoConnectException e) {
            e.printStackTrace(log) ;
            result =  false ;
        } finally {
            oObj.stopAccepting();
            if (acception.isAlive()) {
                acception.interrupt();
            }
        }

        // duplicate acceptor test
        // creating the additional acceptor which listens
        // on the same port

        log.println("___ Testing for accepting on the same port ...") ;

        try {
            dupAcception = new AcceptorThread(dupAcceptor) ;
            dupAcception.start() ;

            try {
                dupAcception.join(1 * 1000) ;
            } catch(InterruptedException e) {}


            if (dupAcception.isAlive()) {
                log.println("Duplicate acceptor is listening ...") ;

                // now trying to accept on the same port as additional
                // acceptor
                acception = new AcceptorThread(oObj) ;
                acception.start() ;

                try {
                    acception.join(3 * 1000) ;
                } catch(InterruptedException e) {}

                if (acception.isAlive()) {
                    oObj.stopAccepting() ;
                    acception.interrupt() ;

                    log.println("Acceptor with the same port must cause"+
                    " an error but didn't") ;
                    result = false ;
                } else {
                    log.println("Accepted call = " + acception.acceptedCall) ;
                    if (acception.ex == null) {
                        //result = false ;
                        log.println("No exception was thrown when trying"+
                         " to listen on the same port") ;
                    } else {
                        if (acception.ex instanceof
                            com.sun.star.connection.AlreadyAcceptingException ||
                            acception.ex instanceof
                            com.sun.star.connection.ConnectionSetupException) {

                            log.println("Rigth exception was thrown when trying"+
                            " to listen on the same port") ;
                        } else {
                            result = false ;
                            log.println("Wrong exception was thrown when trying"+
                            " to listen on the same port :") ;
                            acception.ex.printStackTrace(log) ;
                        }
                    }
                }
            }
        } finally {
            dupAcceptor.stopAccepting() ;
            if (dupAcception.isAlive()) {
                dupAcception.interrupt() ;
            }
        }

        tRes.tested("accept()", result) ;
    }

    /**
    * Starts thread with Acceptor and then calls <code>stopListening</code>
    * method. <p>
    * Has OK status if <code>accept</code> method successfully returns and
    * rises no exceptions.
    */
    public void _stopAccepting() {
        boolean result = true ;


        AcceptorThread acception = new AcceptorThread(oObj) ;

        acception.start() ;

        oObj.stopAccepting() ;

        try {
            acception.join(3 * 1000) ;
        } catch (InterruptedException e) {}

        if (acception.isAlive()) {
            acception.interrupt() ;

            result = false ;
            log.println("Method call haven't returned") ;

        } else {
            if (acception.ex != null) {
                log.println("Exception occured in accept() thread :") ;
                acception.ex.printStackTrace(log) ;
                result = false ;
            } else {
                result = true ;
            }

            if (acception.acceptedCall == null)
                log.println("accept() returned : null") ;
            else
                log.println("accept() returned : " +
                    acception.acceptedCall.getDescription()) ;
        }

        tRes.tested("stopAccepting()", result) ;
    }
}

