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

package complex.imageManager;

import com.sun.star.embed.XStorage;
import com.sun.star.ui.XUIConfigurationPersistence;
import lib.TestParameters;



public class _XUIConfigurationPersistence {


    TestParameters tEnv = null;
    public XUIConfigurationPersistence oObj;
    private XStorage xStore = null;

    public _XUIConfigurationPersistence(TestParameters tEnv, XUIConfigurationPersistence oObj) {

        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    public void before() {
        xStore = (XStorage)tEnv.get("XUIConfigurationStorage.Storage");
    }

    public boolean _reload() {
        try {
            oObj.reload();
        }
        catch(com.sun.star.uno.Exception e) {

        }
        return true;
    }

    public boolean _store() {
        try {
            oObj.store();
        }
        catch(com.sun.star.uno.Exception e) {

        }
        return true;
    }

    public boolean _storeToStorage() {
        boolean result = true;
        try {
            oObj.storeToStorage(xStore);
        }
        catch(com.sun.star.uno.Exception e) {
            result = false;

        }
        return result;
    }

    public boolean _isModified() {
        return !oObj.isModified();
    }

    public boolean _isReadOnly() {
        return  !oObj.isReadOnly();
    }

}
