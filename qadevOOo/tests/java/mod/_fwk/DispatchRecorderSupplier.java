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

package mod._fwk;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.uno.XInterface;

/**
* Test for object that implements the following interfaces :
* <ul>
*  <li><code>com::sun::star::frame::XDispatchRecorderSupplier</code></li>
* </ul><p>
* @see com.sun.star.frame.XDispatchRecorderSupplier
* @see ifc.frame._XDispatchRecorderSupplier
*/
public class DispatchRecorderSupplier extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates instance of
    *  <code>com.sun.star.comp.framework.DispatchRecorderSupplier</code>.
    */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws Exception {

        XInterface oObj = (XInterface)Param.getMSF().createInstance(
                "com.sun.star.comp.framework.DispatchRecorderSupplier");

        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment
}
