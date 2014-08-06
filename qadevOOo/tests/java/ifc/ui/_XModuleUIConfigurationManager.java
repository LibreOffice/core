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

import com.sun.star.container.XIndexAccess;
import com.sun.star.ui.XModuleUIConfigurationManager;
import lib.MultiMethodTest;

public class _XModuleUIConfigurationManager extends MultiMethodTest {

    public XModuleUIConfigurationManager oObj = null;
    private String resourceUrl = null;

    @Override
    public void before() {
        resourceUrl = (String)tEnv.getObjRelation("XModuleUIConfigurationManager.ResourceURL");
    }

    public void _isDefaultSettings() {
        boolean result;
        try {
            result = oObj.isDefaultSettings(resourceUrl);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("'" + resourceUrl + "' is an illegal resource.");
            result = false;
        }
        String notPossibleUrl = "private:resource/menubar/dingsbums";
        try {
            result &= !oObj.isDefaultSettings(notPossibleUrl);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("'" + notPossibleUrl + "' is an illegal resource.");
            result = false;
        }

        tRes.tested("isDefaultSettings()", result);
    }

    public void _getDefaultSettings() {
        boolean result;
        try {
            XIndexAccess xIndexAcc = oObj.getDefaultSettings(resourceUrl);
            result = xIndexAcc != null;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("'" + resourceUrl + "' is an illegal resource.");
            result = false;
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            log.println("No resource '" + resourceUrl + "' available.");
            result = false;
        }
        tRes.tested("getDefaultSettings()", result);
    }

}
