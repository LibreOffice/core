/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Acceptor.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:47:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


