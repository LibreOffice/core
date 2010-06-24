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

package ifc.table;

import lib.MultiMethodTest;
import util.ValueComparer;

import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XTableChart;

/**
* Testing <code>com.sun.star.table.XTableChart</code>
* interface methods :
* <ul>
*  <li><code> getHasColumnHeaders()</code></li>
*  <li><code> setHasColumnHeaders()</code></li>
*  <li><code> getHasRowHeaders()</code></li>
*  <li><code> setHasRowHeaders()</code></li>
*  <li><code> getRanges()</code></li>
*  <li><code> setRanges()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.table.XTableChart
*/
public class _XTableChart extends MultiMethodTest {

    public XTableChart oObj = null;

    /**
     * Sets the property to <code>false</code> and then check it. <p>
     * Has <b> OK </b> status if the method returns <code>false</code>. <p>
     */
    public void _getHasColumnHeaders() {
        oObj.setHasColumnHeaders(false);
        tRes.tested("getHasColumnHeaders()", !oObj.getHasColumnHeaders() );
    } // getHasColumnHeaders()

    /**
     * Sets the property to <code>true</code> and then check it. <p>
     * Has <b> OK </b> status if the method returns <code>true</code>. <p>
     */
    public void _setHasColumnHeaders() {
        oObj.setHasColumnHeaders(true);
        tRes.tested("setHasColumnHeaders()", oObj.getHasColumnHeaders() );
    } // setHasColumnHeaders()

    /**
     * Sets the property to <code>false</code> and then check it. <p>
     * Has <b> OK </b> status if the method returns <code>false</code>. <p>
     */
    public void _getHasRowHeaders() {
        oObj.setHasRowHeaders(false);
        tRes.tested("getHasRowHeaders()", !oObj.getHasRowHeaders() );
    } // getHasRowHeaders()

    /**
     * Sets the property to <code>true</code> and then check it. <p>
     * Has <b> OK </b> status if the method returns <code>true</code>. <p>
     */
    public void _setHasRowHeaders() {
        oObj.setHasRowHeaders(true);
        tRes.tested("setHasRowHeaders()", oObj.getHasRowHeaders() );
    } // setHasRowHeaders()

    CellRangeAddress[] the_Ranges = null;

    /**
     * Test calls the method and stores the range returned. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> valuie. <p>
     */
    public void _getRanges() {
        the_Ranges = oObj.getRanges();
        tRes.tested("getRanges()", the_Ranges != null );
     } // getRanges()

    /**
     * Changes the first range in range array obtained by
     * <code>getRanges</code> method, then set changed array. <p>
     * Has <b> OK </b> status if range array get is the same as was
     * set. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getRanges() </code> : to have initial ranges </li>
     * </ul>
     */
    public void _setRanges() {
        requiredMethod("getRanges()");
        CellRangeAddress[] tmpRanges = oObj.getRanges();
        tmpRanges[0].EndRow = 1;
        oObj.setRanges(tmpRanges);
        tRes.tested("setRanges()", ValueComparer.equalValue(
                                                tmpRanges,oObj.getRanges()));
        oObj.setRanges(the_Ranges);
     } // getRanges()

} // finish class _XTableChartsSupplier



