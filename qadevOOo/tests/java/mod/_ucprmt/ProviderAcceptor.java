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

import com.sun.star.ucb.XRemoteContentProviderAcceptor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ProviderAcceptor extends TestCase {

    @Override
    public TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) throws Exception {

        Object oInterface = Param.getMSF().createInstance
            ("com.sun.star.ucb.RemoteContentProviderAcceptor");
        XInterface oObj = (XInterface) oInterface;

        TestEnvironment tEnv = new TestEnvironment( oObj );

        //adding ObjRelation for XRemoteContentProviderAcceptor
        XRemoteContentProviderAcceptor xRCPA = UnoRuntime.queryInterface(XRemoteContentProviderAcceptor.class,oObj);

        tEnv.addObjRelation("RCPA",xRCPA);

        return tEnv;
    } // finish method getTestEnvironment

}

