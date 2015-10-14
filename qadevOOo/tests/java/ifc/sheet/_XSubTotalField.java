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

import java.util.Arrays;

import com.sun.star.sheet.GeneralFunction;
import com.sun.star.sheet.SubTotalColumn;
import com.sun.star.sheet.XSubTotalField;

/**
* Testing <code>com.sun.star.sheet.XSubTotalField</code>
* interface methods :
* <ul>
*  <li><code> getGroupColumn()</code></li>
*  <li><code> setGroupColumn()</code></li>
*  <li><code> getSubTotalColumns()</code></li>
*  <li><code> setSubTotalColumns()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSubTotalField
*/
public class _XSubTotalField extends MultiMethodTest {

    public XSubTotalField oObj = null;
    public int GroupColumn = 0;
    public SubTotalColumn[] STColumns = null;

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't equal to zero. <p>
    */
    public void _getGroupColumn() {
        GroupColumn = oObj.getGroupColumn();
        tRes.tested("getGroupColumn()", GroupColumn != 0);
    } // end of getGroupColumn

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getSubTotalColumns() {
        STColumns = oObj.getSubTotalColumns();
        tRes.tested("getSubTotalColumns()", STColumns != null);
    } // end of getSubTotalColumns

    /**
    * Test sets new value of the column by which entries are grouped,
    * gets the current value and compares
    * returned value with value that was stored by method
    * <code>getGroupColumn()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getGroupColumn() </code> : to have the current column by
    *  which entries are grouped </li>
    * </ul>
    */
    public void _setGroupColumn() {
        requiredMethod("getGroupColumn()");
        oObj.setGroupColumn(2);
        tRes.tested("setGroupColumn()", GroupColumn != oObj.getGroupColumn());
    } // end of getGroupColumn

    /**
    * Test sets new value using method, gets the current value and compares
    * returned value with value that was stored by method
    * <code>getSubTotalColumns()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getSubTotalColumns() </code> : to have the current value</li>
    * </ul>
    */
    public void _setSubTotalColumns() {
        requiredMethod("getSubTotalColumns()");
        SubTotalColumn[] columns = new SubTotalColumn[2];
        SubTotalColumn column = new SubTotalColumn();
        column.Column=2;
        column.Function=GeneralFunction.AVERAGE;
        columns[0]=column;
        columns[1]=STColumns[0];
        oObj.setSubTotalColumns(columns);
        tRes.tested(
            "setSubTotalColumns()",
            !Arrays.equals(STColumns, oObj.getSubTotalColumns()) );
    } // end of getSubTotalColumns

}  // finish class _XSubTotalField


