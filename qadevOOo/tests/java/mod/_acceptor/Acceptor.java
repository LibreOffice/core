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

package mod._acceptor;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Here <code>com.sun.star.connection.Acceptor</code> service is tested.<p>
* Test allows to run object tests in several threads concurently.
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
    private static String sOfficeHost = null ;

    /**
    * Retrieves host name where StarOffice is started from test
    * parameter <code>'CNCSTR'</code>.
    */
    public void initialize( TestParameters tParam, PrintWriter log ) {
        String cncstr = (String) tParam.get("CNCSTR") ;
        int idx = cncstr.indexOf("host=") + 5 ;
        sOfficeHost = cncstr.substring(idx, cncstr.indexOf(",", idx)) ;
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested. <p>
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
    public synchronized TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log ) throws StatusException {

        XInterface oObj = null;
        XInterface acceptor = null;

        try {
            acceptor = (XInterface)
                ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.connection.Acceptor");
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create object environment", e) ;
        }

        // select the port
        curPort = utils.getNextFreePort(basePort);
        log.println("Choose Port nr: " + curPort);
        oObj = acceptor;

        TestEnvironment tEnv = new TestEnvironment(oObj) ;

        // adding connection string as relation
        tEnv.addObjRelation("XAcceptor.connectStr",
            "socket,host=" + sOfficeHost + ",port=" + curPort) ;

        // adding port number for freeing it.
        tEnv.addObjRelation("Acceptor.Port", new Integer(curPort)) ;

        return tEnv ;
    }

    /**
    * Just clears flag which indicates that port is free now.
    */
    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {

        curPort = ((Integer)tEnv.getObjRelation("Acceptor.Port")).intValue();
    }
}


