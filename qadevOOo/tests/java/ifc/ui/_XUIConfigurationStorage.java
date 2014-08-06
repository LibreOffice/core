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

import com.sun.star.embed.XStorage;
import com.sun.star.ui.XUIConfigurationStorage;
import lib.MultiMethodTest;
/**
* Testing <code>com.sun.star.ui.XUIConfigurationStorage</code>
* interface methods :
* <ul>
*  <li><code> setStorage()</code></li>
*  <li><code> hasStorage()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ui.XUIConfigurationStorage
*/

public class _XUIConfigurationStorage extends MultiMethodTest {

    public XUIConfigurationStorage oObj;
    private XStorage xStore = null;

    @Override
    public void before() {
        xStore = (XStorage)tEnv.getObjRelation("XUIConfigurationStorage.Storage");
    }

    /**
    * Set the object relation <code>XUIConfigurationStorage.Storage</code>
    * as storage.
    * Has <b>OK</b> status if no exception is thrown.
    */
    public void _setStorage() {
        if (!oObj.hasStorage())
            oObj.setStorage(xStore);
        else
            log.println("For initializing this object, the setStorage() method was already called at the object.");
        tRes.tested("setStorage()", true);
    }

    /**
     * Queries object for a storage.
     * Has <b>OK</b> status if <code>hasStorage</code> returns <code>OK</code>
     */
    public void _hasStorage() {
        requiredMethod("setStorage()");
        boolean has = oObj.hasStorage();
        tRes.tested("hasStorage()", has);
    }

}