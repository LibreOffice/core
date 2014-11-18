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

package complex.imageManager;

import com.sun.star.embed.XStorage;
import com.sun.star.ui.XUIConfigurationPersistence;
import lib.TestParameters;



public class _XUIConfigurationPersistence {


    private final TestParameters tEnv;
    private final XUIConfigurationPersistence oObj;
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
