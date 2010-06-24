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

package mod._cnt;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.container.XIndexContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.cnt.CntUnoDataContainer</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XActiveDataSink</code></li>
*  <li> <code>com::sun::star::container::XIndexContainer</code></li>
*  <li> <code>com::sun::star::ucb::XDataContainer</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexReplace</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.io.XActiveDataSink
* @see com.sun.star.container.XIndexContainer
* @see com.sun.star.ucb.XDataContainer
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XIndexReplace
* @see ifc.io._XActiveDataSink
* @see ifc.container._XIndexContainer
* @see ifc.ucb._XDataContainer
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XIndexReplace
*/
public class CntUnoDataContainer extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.cnt.CntUnoDataContainer</code> and adds
    * to it one <code>CntUnoDataContainer</code> instance. <p>
    *     Object relations created :
    * <ul>
    *  <li> <code>'INSTANCE1'</code> for
    *      {@link ifc.container._XIndexContainer},
    *      {@link ifc.container._XIndexReplace} : for this object
    *      elements contained in it are <code>CntUnoDataContainer</code>
    *      instances. (only one relation for one interface thread is now
    *      passed).</li>
    *  <li> <code>'InputStream'</code> for
    *      {@link ifc.io._XActiveDataSink} : a stream to set and get.
    *       </li>
    * </ul>
    */
    protected TestEnvironment createTestEnvironment(
                                    TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        Object relationContainer = null ;
        Object xIn = null;

        try {
            oInterface = xMSF.createInstance
                ( "com.sun.star.ucb.DataContainer" );

            // adding one child container
            XIndexContainer xIC = (XIndexContainer) UnoRuntime.queryInterface
                (XIndexContainer.class, oInterface) ;

            Object child = xMSF.createInstance
                ( "com.sun.star.ucb.DataContainer" );
            xIC.insertByIndex(0, child) ;

            relationContainer = xMSF.createInstance
                ( "com.sun.star.ucb.DataContainer" );

            xIn = xMSF.createInstance
                ( "com.sun.star.io.DataInputStream" );
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        oObj = (XInterface) oInterface;

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("INSTANCE1", relationContainer) ;
        tEnv.addObjRelation("InputStream", xIn) ;

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class BreakIterator

