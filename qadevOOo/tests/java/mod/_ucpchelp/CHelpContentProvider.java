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

package mod._ucpchelp;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class CHelpContentProvider extends TestCase {

    @Override
    public TestEnvironment createTestEnvironment
            ( TestParameters Param,PrintWriter log ) throws Exception {

        XInterface oObj = null;
        Object oInterface = null;
        Object aUCB = null;

        XMultiServiceFactory xMSF = Param.getMSF();
        oInterface = xMSF.createInstance
            ( "com.sun.star.ucb.HelpContentProvider" );
        aUCB = xMSF.createInstance
            ( "com.sun.star.comp.ucb.UniversalContentBroker" );

        oObj = (XInterface) oInterface;

        XContentIdentifierFactory CIF = UnoRuntime.queryInterface(XContentIdentifierFactory.class,aUCB);

        System.out.println("ImplementationName: "+util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        //Adding ObjRelation for XContentProvider
        tEnv.addObjRelation("FACTORY",CIF);
        tEnv.addObjRelation("CONTENT1", "vnd.sun.star.help://swriter?System=WIN&Language=de&Query=text&HitCount=120&Scope=Heading");
        tEnv.addObjRelation("CONTENT2",
            util.utils.getFullTestURL("SwXTextEmbeddedObject.sdw"));

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class FileProvider

