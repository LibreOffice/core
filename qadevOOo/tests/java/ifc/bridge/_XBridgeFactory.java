/*************************************************************************
 *
 *  $RCSfile: _XBridgeFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:16:31 $
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
    * Then stores exception thrown by call if it occured, or
    * return value.
    */
    protected class AcceptorThread extends Thread {
        /**
         * the acceptor
         */
        private XAcceptor acc = null ;
        /**
        * If exception occured during method call it is
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
            if (protocol == null) protocol = "iiop" ;

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

