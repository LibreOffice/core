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

