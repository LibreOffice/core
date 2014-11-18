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

package ifc.frame;

import com.sun.star.frame.XUIControllerRegistration;
import lib.MultiMethodTest;

public class _XUIControllerRegistration extends MultiMethodTest {
    public XUIControllerRegistration oObj = null;
    private String aRegisteredController = null;
    private static final String aController = ".uno:MySecondController";

    @Override
    public void before() {
        aRegisteredController = (String)tEnv.getObjRelation("XUIControllerRegistration.RegisteredController");
    }


    public void _registerController() {
        oObj.registerController(aController, "", "com.sun.star.comp.framework.FooterMenuController");
        oObj.registerController(aRegisteredController, "", "com.sun.star.comp.framework.HeaderMenuController");
        tRes.tested("registerController()", true);
    }

    public void _deregisterController() {
        requiredMethod("registerController()");
        oObj.deregisterController(aController, "");
        tRes.tested("deregisterController()", true);
    }

    public void _hasController() {
        requiredMethod("registerController()");
        requiredMethod("deregisterController()");
        boolean res = oObj.hasController(aRegisteredController, "");
        res &= !oObj.hasController(aController, "");
        tRes.tested("hasController()", res);
    }

}
