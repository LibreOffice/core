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

import com.sun.star.sheet.XViewPane;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XViewPane</code>
* interface methods :
* <ul>
*  <li><code> getFirstVisibleColumn()</code></li>
*  <li><code> setFirstVisibleColumn()</code></li>
*  <li><code> getFirstVisibleRow()</code></li>
*  <li><code> setFirstVisibleRow()</code></li>
*  <li><code> getVisibleRange()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XViewPane
*/
public class _XViewPane extends MultiMethodTest {

    public XViewPane oObj = null;
    int row = 3;
    int col = 5;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal to value that was set
    * by method <code>setFirstVisibleColumn()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setFirstVisibleColumn() </code> : to set specific value
    *  of the first column that is visible in the pane </li>
    * </ul>
    */
    public void _getFirstVisibleColumn() {
        requiredMethod("setFirstVisibleColumn()");
        boolean result = col == oObj.getFirstVisibleColumn();
        tRes.tested("getFirstVisibleColumn()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal to value that was set
    * by method <code>setFirstVisibleRow()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setFirstVisibleRow() </code> : to set specific value of
    *  the first row that is visible in the pane </li>
    * </ul>
    */
    public void _getFirstVisibleRow() {
        requiredMethod("setFirstVisibleRow()");
        boolean result = row == oObj.getFirstVisibleRow();
        tRes.tested("getFirstVisibleRow()", result);
    }

    /**
    * Test just calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setFirstVisibleColumn() {
        oObj.setFirstVisibleColumn(col);
        tRes.tested("setFirstVisibleColumn()", true);
    }

    /**
    * Test just calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setFirstVisibleRow() {
        oObj.setFirstVisibleRow(row);
        tRes.tested("setFirstVisibleRow()", true);
    }

    /**
    * Test calls the method, checks returned value and adds object relation
    * 'DATAAREA' to test environment. <p>
    * Has <b> OK </b> status if returned value isn't null and if start row and
    * start column are equal to values that was set by methods
    * <code>setFirstVisibleRow</code> and <code>setFirstVisibleColumn</code>.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setFirstVisibleRow() </code> : to set specific value of
    *  the first row that is visible in the pane </li>
    *  <li> <code> setFirstVisibleColumn() </code> : to set specific value of
    *  the first column that is visible in the pane </li>
    * </ul>
    */
    public void _getVisibleRange() {
        requiredMethod("setFirstVisibleRow()");
        requiredMethod("setFirstVisibleColumn()");

        CellRangeAddress RA = oObj.getVisibleRange();
        boolean result = RA != null;
        if (result) {
            result &= RA.Sheet == 0;
            result &= RA.StartRow == row;
            result &= RA.StartColumn == col;
            tEnv.addObjRelation("DATAAREA", RA);
        }

        tRes.tested("getVisibleRange()", result);
    }
}

