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

package mod._sfx;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.dbg;
import util.utils;

import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.sfx2.SfxMacroLoader</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::frame::XDispatchProvider</code></li>
 *  <li> <code>com::sun::star::lang::XInitialization</code></li>
 * </ul>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.frame.XDispatchProvider
 * @see com.sun.star.lang.XInitialization
 * @see ifc.frame._XDispatchProvider
 * @see ifc.lang._XInitialization
 */
public class SfxMacroLoader extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.sfx2.SfxMacroLoader</code>. <p>
    *     Object relations created :
    * <ul>
    *  <li> <code>'FrameLoader.URL"'</code> for
    *      {@link ifc.frame._XFrameLoader} </li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) throws Exception {
        XInterface oObj = null;
        Object oInterface = null ;

        //now get the OButtonControl
        oInterface = Param.getMSF().createInstance
                            ("com.sun.star.comp.sfx2.SfxMacroLoader");

        oObj = (XInterface) oInterface ;
        System.out.println("ImplName: "+utils.getImplName(oObj));
        dbg.printInterfaces(oObj);

        log.println( "creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation for XFrameLoader
        tEnv.addObjRelation("XDispatchProvider.URL",
            "macro:tools.UCB.CreateFolder(\"" +
            utils.getOfficeTemp(Param.getMSF()) + "/SfxMacroLoader\")") ;


        return tEnv;
    } // finish method getTestEnvironment

}

