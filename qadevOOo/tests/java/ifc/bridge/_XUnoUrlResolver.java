/*************************************************************************
 *
 *  $RCSfile: _XUnoUrlResolver.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:16:39 $
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
import util.utils;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.connection.ConnectionSetupException;
import com.sun.star.connection.NoConnectException;
import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.bridge.XUnoUrlResolver</code>
* interface methods :
* <ul>
*  <li><code> resolve()</code></li>
* </ul> <p>
* @see com.sun.star.bridge.XUnoUrlResolver
*/
public class _XUnoUrlResolver extends MultiMethodTest {

    // starting port and current port to choose
    static int basePort = 0;
    int curPort = 0;

    public XUnoUrlResolver oObj;

    /**
     * Implementation for providing an instance
     *
     * @see com.sun.star.bridge.XInstanceProvider
     */
    class MyInstanceProvider implements XInstanceProvider {
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
            try {
                return xMSF.createInstance(aInstanceName);
            }
            catch(com.sun.star.uno.Exception e) {
                throw new StatusException("Unexpected exception", e);
            }
        }
    }

    /**
     * Thread for creating a bridge so the resolver can access it
     */
    class BridgeThread extends Thread {
        private XBridgeFactory xBrdgFctr = null;
        private XInstanceProvider xInstProv = null;
        private XAcceptor xAcc = null;
        private String connectString = null;

        public XBridge xBridge = null;

        public BridgeThread(XAcceptor xAcc, XBridgeFactory xBrdgFctr,
            XInstanceProvider xInstProv, String connectString
                ) {
            this.xInstProv = xInstProv;
            this.xBrdgFctr = xBrdgFctr;
            this.xAcc = xAcc;
            this.connectString = connectString;
        }

        public void run() {
            try {
                // create a connection
                XConnection xCon = xAcc.accept(connectString);
                // create a bridge over that conmnection
                xBridge = xBrdgFctr.createBridge(
                                    "MyBridge", "urp", xCon, xInstProv);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                e.printStackTrace(log);
            } catch (com.sun.star.connection.ConnectionSetupException e) {
                e.printStackTrace(log);
            } catch (com.sun.star.connection.AlreadyAcceptingException e) {
                e.printStackTrace(log);
            } catch (com.sun.star.bridge.BridgeExistsException e) {
                e.printStackTrace(log);
            }
        }

    }
    /**
    * Test calls the method using environment property
    * <code>'CNCSTR'</code>. <p>
    * Has <b> OK </b> status if the method successfully returns
    * object that support interface <code>XMultiServiceFactory</code> and
    * no exceptions were thrown. <p>
    * @see com.sun.star.lang.XMultiServiceFactory
    */
    public void _resolve() {
        String connectStr = (String)tParam.get("CNCSTR");
        int pIndex = connectStr.indexOf("port=") + 5;
        connectStr = connectStr.substring(0, pIndex);
        System.out.println("ConnectString: " + connectStr);

        // select the port
        basePort = ((Integer)tEnv.getObjRelation("PORT")).intValue();
        curPort = utils.getNextFreePort(basePort);
        log.println("Choose Port nr: " + curPort);

        connectStr += curPort;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

            // get the bridge factory
            XBridgeFactory xBrdgFctr = (XBridgeFactory)
                        UnoRuntime.queryInterface(XBridgeFactory.class,
                                    tEnv.getObjRelation("BRIDGEFACTORY"));

            // get the acceptor
            XAcceptor xAcc = (XAcceptor)UnoRuntime.queryInterface(
                    XAcceptor.class, tEnv.getObjRelation("ACCEPTOR"));

            // instance provider
            XInstanceProvider xInstProv = new MyInstanceProvider(xMSF);
            // thread for providing a bridge
            BridgeThread brThread = new BridgeThread(xAcc, xBrdgFctr,
                                            xInstProv, connectStr);
            brThread.start();

            try {
                Thread.sleep(500);
            }
            catch(java.lang.InterruptedException e) {}
            // get an instance from the remote
            Object obj = oObj.resolve(
                    "uno:" + connectStr + ";urp;com.sun.star.lang.ServiceManager");
            // got the instance?
            XMultiServiceFactory oMSF = (XMultiServiceFactory)
                    UnoRuntime.queryInterface(XMultiServiceFactory.class, obj);

            if (brThread.isAlive())
                brThread.interrupt();

            tRes.tested("resolve()", oMSF != null);
        } catch (NoConnectException e) {
            log.println("Unexpected exception thrown " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        } catch (ConnectionSetupException e) {
            log.println("Unexpected exception thrown " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        } catch (IllegalArgumentException e) {
            log.println("Unexpected exception thrown " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }
    }
}
