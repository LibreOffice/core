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

package mod._fileacc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.ucb.SimpleFileAccess</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li><code>com::sun::star::ucb.XSimpleFileAccess</code></li>
* </ul>
* This object test <b> can </b> be run in several
* threads concurently.
* @see com.sun.star.ucb.XSimpleFileAccess
* @see com.sun.star.ucb.SimpleFileAccess
* @see ifc.ucb._XSimpleFileAccess
*/
public class SimpleFileAccess extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates service <code>com.sun.star.ucb.SimpleFileAccess</code>.
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;
        Object oIH = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ("com.sun.star.comp.ucb.SimpleFileAccess");
            oIH = xMSF.createInstance("com.sun.star.sdb.InteractionHandler");
        } catch( com.sun.star.uno.Exception e ) {
            log.println("SimpleFileAccess Service not available" );
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for CoreReflection object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("InteractionHandler", oIH);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SimpleFileAccess

