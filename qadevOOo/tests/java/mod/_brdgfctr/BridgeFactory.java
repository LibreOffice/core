/*************************************************************************
 *
 *  $RCSfile: BridgeFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:34:50 $
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

package mod._brdgfctr;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Tests <code>com.sun.star.bridge.BridgeFactory</code>
* service. <p>
* @see com.sun.star.bridge.XBridgeFactory
* @see com.sun.star.lang.XComponent
* @see ifc.bridge._XBridgeFactory
* @see ifc.lang._XComponent
*/
public class BridgeFactory extends TestCase {

    /**
    * Retrieves host name where StarOffice is started from test
    * parameter <code>'CNCSTR'</code>.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        String cncstr = (String) tParam.get("CNCSTR") ;
        int idx = cncstr.indexOf("host=") + 5 ;
        sOfficeHost = cncstr.substring(idx, cncstr.indexOf(",", idx)) ;
    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
    }

    /**
    * Acceptor chooses the first port after <code>basePort</code>
    * which is free.
    */
    protected static final int basePort = 50003 ;
    private int curPort ;
    private static String sOfficeHost = null ;

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Just creates <code>com.sun.star.bridge.BridgeFactory</code>
     * service as object to be tested.
     */
    public synchronized TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log ) throws StatusException {

        XInterface oObj = null ;

        try {
            oObj = (XInterface) ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.bridge.BridgeFactory") ;
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create object environment", e) ;
        }

        if (oObj == null)
            throw new StatusException("Can't create service",
                new NullPointerException());

        TestEnvironment tEnv = new TestEnvironment(oObj) ;

        // select the port
        curPort = utils.getNextFreePort(basePort);
        log.println("Choose Port nr: " + curPort);

        // adding connection string as relation
        tEnv.addObjRelation("CNNCTSTR",
            "socket,host=" + sOfficeHost + ",port=" + curPort) ;

        // adding port number for freeing it.
        tEnv.addObjRelation("Connector.Port", new Integer(curPort)) ;

        return tEnv ;
    }

    /**
    * Just clears flag which indicates that port is free now.
    */
    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {
        curPort = ((Integer)tEnv.getObjRelation("Connector.Port")).intValue() ;
    }
}


