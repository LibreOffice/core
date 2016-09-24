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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XViewFreezable;

/**
* Testing <code>com.sun.star.sheet.XViewFreezable</code>
* interface methods :
* <ul>
*  <li><code> hasFrozenPanes()</code></li>
*  <li><code> freezeAtPosition()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XViewFreezable
*/
public class _XViewFreezable extends MultiMethodTest {

    public XViewFreezable oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _freezeAtPosition() {
        oObj.freezeAtPosition(2, 2);
        tRes.tested("freezeAtPosition()", true);
    }

    /**
    * Test calls the method, checks returned value, unfreezes panes, calls the
    * method and checks returned value again. <p>
    * Has <b> OK </b> status if returned value is true after first call and
    * returned value is false after second. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> freezeAtPosition() </code> : to freeze panes with the
    *   specified number of columns and rows </li>
    * </ul>
    */
    public void _hasFrozenPanes() {
        requiredMethod("freezeAtPosition()");
        boolean result = oObj.hasFrozenPanes();
        oObj.freezeAtPosition(0,0);
        result &= !oObj.hasFrozenPanes();
        tRes.tested("hasFrozenPanes()", result);
    }


}  // finish class _XViewFreezable


