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

package mod._uuresolver.uno;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

public class UnoUrlResolver extends TestCase {


    protected TestEnvironment createTestEnvironment(TestParameters tParam,
            PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        try {
            XInterface xInt = (XInterface)xMSF.createInstance(
                    "com.sun.star.bridge.UnoUrlResolver");
            TestEnvironment tEnv = new TestEnvironment(xInt);
            Object oBFctr = xMSF.createInstance(
                                        "com.sun.star.bridge.BridgeFactory");
            tEnv.addObjRelation("BRIDGEFACTORY", oBFctr);

            Object oAcctr = xMSF.createInstance(
                                        "com.sun.star.connection.Acceptor") ;
            tEnv.addObjRelation("ACCEPTOR", oAcctr);

            int port = util.utils.getNextFreePort(20004);
            tEnv.addObjRelation("PORT", new Integer(port));

            return tEnv;
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

    }
}
