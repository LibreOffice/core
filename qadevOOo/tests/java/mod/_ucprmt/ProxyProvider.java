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

package mod._ucprmt;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ProxyProvider extends TestCase {

    @Override
    public TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log ) throws Exception {

        XMultiServiceFactory xMSF = Param.getMSF();
        XInterface oObj = (XInterface) xMSF.createInstance
            ("com.sun.star.ucb.RemoteProxyContentProvider");
        XContentIdentifierFactory cntIDFac = UnoRuntime.queryInterface
            (XContentIdentifierFactory.class, oObj) ;

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation for XContentProvider
        tEnv.addObjRelation("FACTORY", cntIDFac) ;
        tEnv.addObjRelation("CONTENT1","vnd.sun.star.pkg:///user/work");
        tEnv.addObjRelation("CONTENT1","vnd.sun.star.wfs://");

        tEnv.addObjRelation("NoCONTENT",Boolean.TRUE);

        return tEnv;
    } // finish method getTestEnvironment

}

