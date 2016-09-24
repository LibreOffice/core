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

import com.sun.star.sheet.XSheetConditionalEntry;

/**
* Testing <code>com.sun.star.sheet.XSheetConditionalEntry</code>
* interface methods :
* <ul>
*  <li><code> getStyleName()</code></li>
*  <li><code> setStyleName()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetConditionalEntry
*/
public class _XSheetConditionalEntry extends MultiMethodTest {

    public XSheetConditionalEntry oObj = null;
    public String StyleName = null;

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getStyleName() {
        StyleName = oObj.getStyleName();
        tRes.tested("getStyleName()", StyleName != null);
    }

    /**
    * Test sets new value of style name, gets current style name and compares
    * returned value with value that was stored by method <code>getStyleName()
    * </code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getStyleName() </code> : to have current style name </li>
    * </ul>
    */
    public void _setStyleName() {
        requiredMethod("getStyleName()");
        oObj.setStyleName("Heading");
        tRes.tested("setStyleName()", !StyleName.equals(oObj.getStyleName()));
    }

}  // finish class _XSheetConditionalEntry


