/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XUIControllerRegistration.java,v $
 * $Revision: 1.5 $
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

package ifc.frame;

import com.sun.star.frame.XUIControllerRegistration;
import lib.MultiMethodTest;

public class _XUIControllerRegistration extends MultiMethodTest {
    public XUIControllerRegistration oObj = null;
    private String aRegisteredController = null;
    private String aController = ".uno:MySecondController";

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
