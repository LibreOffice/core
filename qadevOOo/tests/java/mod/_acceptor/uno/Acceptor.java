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

package mod._acceptor.uno;

import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

/**
* Here <code>com.sun.star.connection.Acceptor</code> service is tested.<p>
* Test allows to run object tests in several threads concurrently.
* @see com.sun.star.connection.Acceptor
* @see com.sun.star.connection.XAcceptor
* @see com.sun.star.connection.XConnector
* @see ifc.connection._XAcceptor
*/
public class Acceptor extends TestCase {
    /**
    * Acceptor chooses the first port after <code>basePort</code>
    * which is free.
    */
    protected static final int basePort = 10000;
    private int curPort ;
    private String sOfficeHost = null ;

    /**
    * Retrieves host name where StarOffice is started from test
    * parameter <code>'CONNECTION_STRING'</code>.
    */
    @Override
    public void initialize( TestParameters tParam, PrintWriter log ) {
        String cncstr = (String) tParam.get("CONNECTION_STRING") ;
        int idx = cncstr.indexOf("host=") + 5 ;
        sOfficeHost = cncstr.substring(idx, cncstr.indexOf(",", idx)) ;
    }

    /**
     * Creating a TestEnvironment for the interfaces to be tested. <p>
     * Creates <code>Acceptor</code> service and passed as relation
     * connection string where port for accepting is unique among
     * different object test threads. <p>
     * The following object relations are created :
     * <ul>
     * <li> <code>'XAcceptor.connectStr'</code> : String variable for
     *   <code>XAcceptor</code> interface test. Has the following format :
     *   <code>'socket,host=<SOHost>,port=<UniquePort>' where <SOHost> is
     *   the host where StarOffice is started. </li>
     * <li> <code>'Acceptor.Port'</code> : Integer value which specifies
     *   port on which Acceptor must listen, and which is required
     *   when disposing environment, to free this port number. </li>
     * <ul>
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface acceptor = (XInterface)
            Param.getMSF().createInstance
            ("com.sun.star.connection.Acceptor");

        // select the port
        curPort = utils.getNextFreePort(basePort);
        log.println("Choose Port nr: " + curPort);

        TestEnvironment tEnv = new TestEnvironment(acceptor) ;

        // adding connection string as relation
        tEnv.addObjRelation("XAcceptor.connectStr",
            "socket,host=" + sOfficeHost + ",port=" + curPort) ;

        // adding port number for freeing it.
        tEnv.addObjRelation("Acceptor.Port", Integer.valueOf(curPort)) ;

        return tEnv ;
    }

    /**
    * Just clears flag which indicates that port is free now.
    */
    @Override
    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {

        curPort = ((Integer)tEnv.getObjRelation("Acceptor.Port")).intValue();
    }
}