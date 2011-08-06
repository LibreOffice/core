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
    * <p>Has <b> OK </b> status if if values are equal. <p>
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


