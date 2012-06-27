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
            XServiceInfo xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, o);
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