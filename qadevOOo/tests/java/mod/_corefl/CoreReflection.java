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

package mod._corefl;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.reflection.CoreReflection</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::reflection::XIdlReflection</code></li>
* </ul>
* @see com.sun.star.reflection.XIdlReflection
* @see ifc.reflection._XIdlReflection
*/
public class CoreReflection extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates <code>com.sun.star.reflection.CoreReflection</code> service.
    */
    @Override
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws Exception {
        XMultiServiceFactory xMSF = Param.getMSF();
        Object oInterface = xMSF.createInstance
                ("com.sun.star.reflection.CoreReflection");
        XInterface oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for CoreReflection object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class CoreReflection

