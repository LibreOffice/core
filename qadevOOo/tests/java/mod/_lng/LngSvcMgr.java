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

package mod._lng;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.linguistic2.LinguServiceManager</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
 *  <li> <code>com::sun::star::linguistic2::XLinguServiceManager</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 *
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.linguistic2.XLinguServiceManager
 * @see ifc.lang._XComponent
 * @see ifc.lang._XServiceInfo
 * @see ifc.linguistic2._XLinguServiceManager
 */
public class LngSvcMgr extends TestCase {

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.linguistic2.LinguServiceManager</code>.
     */
    @Override
    public TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log )
            throws Exception {

        XMultiServiceFactory xMSF = Param.getMSF();
        XInterface oObj = (XInterface)xMSF.createInstance
                ("com.sun.star.linguistic2.LinguServiceManager");

        String Iname = util.utils.getImplName(oObj);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    }


}    // finish class LngSvcMgr

