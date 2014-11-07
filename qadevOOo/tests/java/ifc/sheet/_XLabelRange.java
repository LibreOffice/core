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
import util.ValueComparer;

import com.sun.star.sheet.XLabelRange;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XLabelRange</code>
* interface methods :
* <ul>
*  <li><code> getLabelArea()</code></li>
*  <li><code> setLabelArea()</code></li>
*  <li><code> getDataArea()</code></li>
*  <li><code> setDataArea()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XLabelRange
*/
public class _XLabelRange extends MultiMethodTest {

    public XLabelRange oObj = null;
    public CellRangeAddress setDAddress = null;
    public CellRangeAddress setLAddress = null;

    /**
    * Test creates and stores <code>CellRangeAddress</code>, calls the method.
    * <p>Has <b> OK </b> status if the method successfully returns. <p>
    * @see com.sun.star.table.CellRangeAddress
    */
    public void _setDataArea() {
        int nr = Thread.activeCount();
        setDAddress = new CellRangeAddress((short)1, nr, 1, nr, 8);
        oObj.setDataArea(setDAddress);
        tRes.tested("setDataArea()", true);
    }

    /**
    * Test creates and stores <code>CellRangeAddress</code>, calls the method.
    * <p>Has <b> OK </b> status if the method successfully returns. <p>
    * @see com.sun.star.table.CellRangeAddress
    */
    public void _setLabelArea() {
        int nr = Thread.activeCount();
        setLAddress = new CellRangeAddress((short)1, nr, 0, nr, 0);
        oObj.setLabelArea(setLAddress);
        tRes.tested("setLabelArea()", true);
    }

    /**
    * Test calls the method and compares returned value with value that was set.
    * <p>Has <b> OK </b> status if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setDataArea() </code> : to have address of the cell range for
    *  which the labels are valid</li>
    * </ul>
    */
    public void _getDataArea() {
        requiredMethod("setDataArea()");
        CellRangeAddress gA = oObj.getDataArea();
        tRes.tested("getDataArea()", ValueComparer.equalValue(gA, setDAddress));
    }

    /**
    * Test calls the method and compares returned value with value set before.
    * <p>Has <b> OK </b> status if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setLabelArea() </code>: to have the cell range that contains
    *  the labels</li>
    * </ul>
    */
    public void _getLabelArea() {
        requiredMethod("setLabelArea()");
        CellRangeAddress gA1 = oObj.getLabelArea();
        tRes.tested("getLabelArea()",
            ValueComparer.equalValue(gA1, setLAddress));
    }
}  // finish class _XLabelRange


