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
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
 */
public class StatusBarControllerFactory extends TestCase {
    XInterface oObj = null;

    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        TestEnvironment tEnv = null;
        XMultiServiceFactory xMSF = tParam.getMSF();
        try {
            oObj = (XInterface)xMSF.createInstance("com.sun.star.comp.framework.StatusBarControllerFactory");

            tEnv = new TestEnvironment(oObj);
        }
        catch(com.sun.star.uno.Exception e) {
            throw new StatusException("Cannot create test object 'StatusBarControllerFactory'", e);
        }
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, xMSF);
        try {
            Object o = xProp.getPropertyValue("DefaultContext");
            XComponentContext xContext = UnoRuntime.queryInterface(XComponentContext.class, o);
            tEnv.addObjRelation("DC", xContext);
        }
        catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Cannot get the 'DefaultContext' for XMultiComponentFactory test.");
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Cannot get the 'DefaultContext' for XMultiComponentFactory test.");
            e.printStackTrace(log);
        }
        return tEnv;
    }
}


