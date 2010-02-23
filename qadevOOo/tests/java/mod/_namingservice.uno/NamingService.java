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

package mod._namingservice.uno;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
* Test for object which is represented by service
* <code>com.sun.star.uno.NamingService</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::uno::XNamingService</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.uno.XNamingService
* @see com.sun.star.uno.NamingService
* @see ifc.uno._XNamingService
*/
public class NamingService extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.uno.NamingService</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XNamingService.RegisterObject'</code> for
    *      {@link ifc.uno._XNamingService} : an object
    *      which can be registered. Here it is an instance
    *      of <code>com.sun.star.lang.ServiceManager</code>
    *      service.</li>
    * </ul>
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = null;

        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ( "com.sun.star.uno.NamingService" );
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // objRelation for XNamingService as object to be registered
        try {
            tEnv.addObjRelation("XNamingService.RegisterObject",
                xMSF.createInstance
                ("com.sun.star.lang.ServiceManager")) ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't create object relation") ;
            throw new StatusException("Can't create object relation", e) ;
        }

        return tEnv;
    } // finish method getTestEnvironment

}

