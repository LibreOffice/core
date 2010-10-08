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

package ifc.ui;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import lib.MultiMethodTest;

public class _XModuleUIConfigurationManagerSupplier extends MultiMethodTest {

    public XModuleUIConfigurationManagerSupplier oObj;

    public void _getUIConfigurationManager() {
        String configManagerName = null;
        String implementationName = null;
        try {
            configManagerName = (String)tEnv.getObjRelation("XModuleUIConfigurationManagerSupplier.ConfigurationManager");
            implementationName = (String)tEnv.getObjRelation("XModuleUIConfigurationManagerSupplier.ConfigManagerImplementationName");

            // get a config manager for the StartModule
            Object o = oObj.getUIConfigurationManager(configManagerName);
            XServiceInfo xServiceInfo = (XServiceInfo)UnoRuntime.queryInterface(XServiceInfo.class, o);
            String impName = xServiceInfo.getImplementationName();
            boolean result = impName.equals(implementationName);
            if (!result) {
                log.println("Returned implementation was '" + impName +
                    "' but should have been '" + implementationName + "'");
            }
            tRes.tested("getUIConfigurationManager()", result);
            return;
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            log.println("Could not get a configuration manager called '" + configManagerName + "'");
            e.printStackTrace(log);
        }
        tRes.tested("getUIConfigurationManager()", false);
    }
}