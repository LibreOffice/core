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

package mod._bridgefac.uno;

import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

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
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null ;

        try {
            oObj = (XInterface)
                    ((XMultiServiceFactory)Param.getMSF()).createInstance
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