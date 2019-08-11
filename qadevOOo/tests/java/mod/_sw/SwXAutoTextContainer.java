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

package mod._sw;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XAutoTextContainer;
import com.sun.star.uno.UnoRuntime;

/**
* Test for object which is represented by service
* <code>com.sun.star.text.AutoTextContainer</code>.<p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::text::XAutoTextContainer</code></li>
* </ul>
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.text.XAutoTextContainer
* @see com.sun.star.text.AutoTextContainer
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
* @see ifc.text._XAutoTextContainer
*/
public class SwXAutoTextContainer extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.text.AutoTextContainer</code>.<p>
    */
    @Override
    public TestEnvironment createTestEnvironment(
            TestParameters Param, PrintWriter log ) throws Exception {
        XAutoTextContainer  oContainer = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating an AutoTextContainer");
        XMultiServiceFactory myMSF = Param.getMSF();
        Object oInst = myMSF.createInstance
            ("com.sun.star.text.AutoTextContainer");
        oContainer = UnoRuntime.queryInterface
            (XAutoTextContainer.class,oInst);

        TestEnvironment tEnv = new TestEnvironment(oContainer);

        return tEnv;
    }

}    // finish class SwXAutoTextcontainer

