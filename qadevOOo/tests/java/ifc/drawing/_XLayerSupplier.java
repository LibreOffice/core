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

package ifc.drawing;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.drawing.XLayerManager;
import com.sun.star.drawing.XLayerSupplier;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.drawing.XLayerSupplier</code>
* interface methods :
* <ul>
*  <li><code> getLayerManager()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.drawing.XLayerSupplier
*/
public class _XLayerSupplier extends MultiMethodTest{
    public XLayerSupplier oObj = null;

    /**
    * Gets the manager and it is queried for <code>XLayerManager</code>. <p>
    * Has <b> OK </b> status if returned value is successfully
    * queried for <code>XLayerManager</code>. <p>
    */
    public void _getLayerManager(){
        XNameAccess oNA =null;
        XLayerManager oLM = null;
        boolean result = true;

        log.println("testing getLayerManager() ... ");

        oNA = oObj.getLayerManager();
        oLM = UnoRuntime.queryInterface ( XLayerManager.class, oNA);
        result = oLM != null;

        tRes.tested("getLayerManager()", result);
    } // finished test

} // end XLayerSupplier

