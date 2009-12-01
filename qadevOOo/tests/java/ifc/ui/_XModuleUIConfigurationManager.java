/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XModuleUIConfigurationManager.java,v $
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

package ifc.ui;

import com.sun.star.container.XIndexAccess;
import com.sun.star.ui.XModuleUIConfigurationManager;
import lib.MultiMethodTest;

public class _XModuleUIConfigurationManager extends MultiMethodTest {

    public XModuleUIConfigurationManager oObj = null;
    private String resourceUrl = null;

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
