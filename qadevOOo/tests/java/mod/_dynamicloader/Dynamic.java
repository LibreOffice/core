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

package mod._dynamicloader;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.loader.Dynamic</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::loader::XImplementationLoader</code></li>
*  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurrently.
* @see com.sun.star.loader.XImplementationLoader
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.loader.Dynamic
* @see ifc.loader._XImplementationLoader
* @see ifc.lang._XServiceInfo
*/
public class Dynamic extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates service
    * <code>com.sun.star.loader.Dynamic</code>.
    */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws Exception {
        XMultiServiceFactory xMSF = Param.getMSF();
        Object oInterface = xMSF.createInstance( "com.sun.star.loader.Dynamic" );
        XInterface oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relations for XImplementationLoader

        String loader = "com.sun.star.loader.SharedLibrary";
        tEnv.addObjRelation("ImplementationLoader", loader) ;

        String url = "servicename=com.sun.star.loader.SharedLibrary,link=mylink";
        tEnv.addObjRelation("ImplementationUrl", url) ;

        tEnv.addObjRelation("ImplementationName", "com.sun.star.io.Pipe") ;

        return tEnv;
    } // finish method getTestEnvironment

}

