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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.uno.XInterface;

/**
* Test for object that implements the following interfaces :
* <ul>
*  <li><code>com::sun::star::frame::XDispatchProvider</code></li>
*  <li><code>com::sun::star::lang::XInitialization</code></li>
* </ul><p>
* @see com.sun.star.frame.XDispatchProvider
* @see com.sun.star.lang.XInitialization
* @see ifc.frame._XDispatchProvider
* @see ifc.lang._XInitialization
*/
public class JobHandler extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws Exception {
        boolean serviceRegistered = false;

        try {
            Object obj = Param.getMSF().createInstance("test.Job");
            serviceRegistered = obj != null;
        } catch(com.sun.star.uno.Exception e) {}

        log.println("Service test.Job is "
            + (serviceRegistered ? "already" : "not yet")  + " registered.");
        if (! serviceRegistered){
            String message = "You have to register 'test.Job' before office is started.\n";
            message += "Please run '$OFFICEPATH/program/pkgchk $DOCPTH/qadevlibs/JobExecutor.jar'";
            throw new StatusException(message, new Exception());
        }

        XInterface oObj = (XInterface)Param.getMSF().createInstance(
            "com.sun.star.comp.framework.jobs.JobDispatch");

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("XDispatchProvider.URL", "vnd.sun.star.job:alias=TestJob");
        return tEnv;
    } // finish method getTestEnvironment

}
