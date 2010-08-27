/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package mod._uuresolver;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.XInterface;

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

            String os = System.getProperty("OS");
            System.out.println("OS: " + os);
            int port = 20004;
            if (os.equalsIgnoreCase("wntmsci"))
              port = 20004;
            if (os.equalsIgnoreCase("unxsols"))
              port = 20004;
            if (os.equalsIgnoreCase("unxsoli"))
              port = 30004;
            if (os.equalsIgnoreCase("unxlngi"))
              port = 20006;

            tEnv.addObjRelation("PORT", new Integer(port));

            return tEnv;
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

    }
}
