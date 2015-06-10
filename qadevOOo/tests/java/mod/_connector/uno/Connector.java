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

package mod._connector.uno;

import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

/**
* Test for object which is represented by service
* <code>com.sun.star.connection.Connector</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::connection::XConnector</code></li>
* </ul>
* Can be run in several threads.
* @see com.sun.star.connection.XConnector
* @see ifc.connection._XConnector
*/
public class Connector extends TestCase {

    /**
    * Acceptor chooses the first port after <code>basePort</code>
    * which is free.
    */
    protected static final int basePort = 10000 ;
    private int curPort ;
    private static String sOfficeHost = null ;

    /**
    * Retrieves host name where StarOffice is started from test
    * parameter <code>'CONNECTION_STRING'</code>.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        String cncstr = (String) tParam.get("CONNECTION_STRING") ;
        int idx = cncstr.indexOf("host=") + 5 ;
        sOfficeHost = cncstr.substring(idx, cncstr.indexOf(",", idx)) ;
    }

    /**
    * Does nothing.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {

    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Just creates service <code>com.sun.star.connection.Connector</code>
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface connector = (XInterface)
            Param.getMSF().createInstance
                                ("com.sun.star.connection.Connector") ;

        TestEnvironment tEnv = new TestEnvironment(connector) ;

        // select the port
        curPort = utils.getNextFreePort(basePort);
        log.println("Choose Port nr: " + curPort);

        // adding connection string as relation
        tEnv.addObjRelation("XConnector.connectStr",
            "socket,host=" + sOfficeHost + ",port=" + curPort) ;

        // adding port number for freeing it.
        tEnv.addObjRelation("Connector.Port", Integer.valueOf(curPort)) ;

        return tEnv ;
    }

    /**
    * Just clears flag which indicates that port is free now.
    */
    @Override
    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {

        curPort = ((Integer)tEnv.getObjRelation("Connector.Port")).intValue();
    }
}