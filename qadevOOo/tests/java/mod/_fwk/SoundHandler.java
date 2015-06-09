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
import com.sun.star.util.URL;
import util.utils;
/**
* Test for object that implements the following interfaces :
* <ul>
*  <li><code>com::sun::star::frame::XDispatch</code></li>
*  <li><code>com::sun::star::frame::XNotifyingDispatch</code></li>
* </ul><p>
* @see com.sun.star.frame.XDispatch
* @see com.sun.star.frame.XNotifyingDispatch
* @see ifc.frame._XDispatch
* @see ifc.frame._XNotifyingDispatch
*/
public class SoundHandler extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws Exception {

        XInterface oObj = (XInterface)Param.getMSF().createInstance(
                "com.sun.star.frame.ContentHandler");

        TestEnvironment tEnv = new TestEnvironment( oObj );
        URL dispURL = utils.parseURL(Param.getMSF(),
                                            utils.getFullTestURL("ok.wav"));
        System.out.println("DISPATCHURL: "+ dispURL.Complete);

        tEnv.addObjRelation("XDispatch.URL", dispURL);
        return tEnv;
    } // finish method getTestEnvironment

}
