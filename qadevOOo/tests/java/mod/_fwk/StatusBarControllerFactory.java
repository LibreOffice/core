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
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        try {
            oObj = (XInterface)xMSF.createInstance("com.sun.star.comp.framework.StatusBarControllerFactory");

            tEnv = new TestEnvironment(oObj);
        }
        catch(com.sun.star.uno.Exception e) {
            throw new StatusException("Cannot create test object 'StatusBarControllerFactory'", e);
        }
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xMSF);
        try {
            Object o = xProp.getPropertyValue("DefaultContext");
            XComponentContext xContext = (XComponentContext)UnoRuntime.queryInterface(XComponentContext.class, o);
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


