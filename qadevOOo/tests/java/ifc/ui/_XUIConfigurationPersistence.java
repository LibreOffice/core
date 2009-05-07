/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XUIConfigurationPersistence.java,v $
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

import com.sun.star.embed.XStorage;
import com.sun.star.ui.XUIConfigurationPersistence;
import lib.MultiMethodTest;

public class _XUIConfigurationPersistence extends MultiMethodTest {

    public XUIConfigurationPersistence oObj;
    private XStorage xStore = null;

    public void before() {
        xStore = (XStorage)tEnv.getObjRelation("XUIConfigurationStorage.Storage");
    }

    public void _reload() {
        try {
            oObj.reload();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
        }
        tRes.tested("reload()", true);
    }

    public void _store() {
        try {
            oObj.store();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
        }
        tRes.tested("store()", true);
    }

    public void _storeToStorage() {
        boolean result = true;
        try {
            oObj.storeToStorage(xStore);
        }
        catch(com.sun.star.uno.Exception e) {
            result = false;
            e.printStackTrace(log);
        }
        tRes.tested("storeToStorage()", result);
    }

    public void _isModified() {
        tRes.tested("isModified()", !oObj.isModified());
    }

    public void _isReadOnly() {
        tRes.tested("isReadOnly()", !oObj.isReadOnly());
    }

}
