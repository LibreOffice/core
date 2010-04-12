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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ui.XUIConfigurationStorage
*/

public class _XUIConfigurationStorage extends MultiMethodTest {

    public XUIConfigurationStorage oObj;
    private XStorage xStore = null;

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