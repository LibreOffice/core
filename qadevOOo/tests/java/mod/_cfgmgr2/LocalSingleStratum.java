/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LocalSingleStratum.java,v $
 * $Revision: 1.4 $
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
package mod._cfgmgr2;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XStringSubstitution;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;


public class LocalSingleStratum extends TestCase {
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XInterface oObj = null;
        String AdminURL = "";

        try {
            Object[] args = new Object[1];
            XStringSubstitution sts = createStringSubstitution(
                                              (XMultiServiceFactory) tParam.getMSF());
            String userURL = sts.getSubstituteVariableValue("$(user)");
            args[0] = userURL + "/registry";
            AdminURL = sts.getSubstituteVariableValue("$(inst)") +
                       "/share/registry";
            log.println("Using: " + args[0]);
            oObj = (XInterface) ((XMultiServiceFactory) tParam.getMSF()).createInstanceWithArguments(
                           "com.sun.star.comp.configuration.backend.LocalSingleStratum",
                           args);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        log.println("Implementation name: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("NoAdmin", AdminURL);

        return tEnv;
    }

    public static XStringSubstitution createStringSubstitution(XMultiServiceFactory xMSF) {
        Object xPathSubst = null;

        try {
            xPathSubst = xMSF.createInstance(
                                 "com.sun.star.util.PathSubstitution");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        if (xPathSubst != null) {
            return (XStringSubstitution) UnoRuntime.queryInterface(
                           XStringSubstitution.class, xPathSubst);
        } else {
            return null;
        }
    }
}
