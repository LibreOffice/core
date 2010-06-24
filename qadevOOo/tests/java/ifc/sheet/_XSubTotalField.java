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
            !STColumns.equals(oObj.getSubTotalColumns()) );
    } // end of getSubTotalColumns

}  // finish class _XSubTotalField


