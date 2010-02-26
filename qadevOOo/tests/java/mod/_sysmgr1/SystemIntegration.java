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

package mod._sysmgr1;

import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

public class SystemIntegration extends TestCase {

    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        XInterface oObj = null;
        Object[] args = new Object[0];

        try {

            oObj = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                      .createInstance("com.sun.star.comp.configuration.backend.SystemIntegration");
            XInitialization xInit = (XInitialization) UnoRuntime.queryInterface(XInitialization.class,oObj);
            xInit.initialize(args);

        } catch (com.sun.star.uno.Exception e) {
        }

        log.println("Implementation name: "+ util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        //objRelation for XBackend
        tEnv.addObjRelation("noUpdate", "SystemIntegrationManager: No Update Operation allowed, Read Only access -- OK");

        return tEnv;
    }

}
