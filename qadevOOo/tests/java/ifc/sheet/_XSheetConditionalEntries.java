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

import com.sun.star.beans.PropertyValue;
import com.sun.star.sheet.ConditionOperator;
import com.sun.star.sheet.XSheetConditionalEntries;
import com.sun.star.table.CellAddress;

/**
* Testing <code>com.sun.star.sheet.XSheetConditionalEntries</code>
* interface methods :
* <ul>
*  <li><code> addNew()</code></li>
*  <li><code> removeByIndex()</code></li>
*  <li><code> clear()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetConditionalEntries
*/
public class _XSheetConditionalEntries extends MultiMethodTest {
    public XSheetConditionalEntries oObj = null;
    int nNum = 0;

    /**
    * Test adds a conditional entry to the format. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _addNew() {
        nNum = oObj.getCount();
        oObj.addNew( Conditions(4) );
        boolean res = (nNum + 1) == oObj.getCount();

        tRes.tested("addNew()", res);
    }

    /**
    * Test calls the method and checks number of conditional entries in
    * collection. <p>
    * Has <b> OK </b> status if number of conditional entries in co0llection
    * after method call is equal zero. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addNew() </code> : to have one conditional entry in
    *  collection at least </li>
    * </ul>
    */
    public void _clear() {
        requiredMethod("removeByIndex()");
        oObj.clear();
        int anz = oObj.getCount();
        tRes.tested("clear()", anz == 0);
    }

    /**
    * Test adds a conditional entry, removes entry with index 0
    * and compares number of entries after adding to number of entries after
    * removing. <p>
    * Has <b> OK </b> status if number of entries after adding is greater
    * than number of entries after removing. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> clear() </code> : to be sure that collection hasn't
    *  elements </li>
    * </ul>
    */
    public void _removeByIndex() {
        requiredMethod("addNew()");
        oObj.removeByIndex(0);
        int pastNum = oObj.getCount();
        tRes.tested("removeByIndex()", pastNum == nNum);
    }

    /**
    * Method creates array of property value for conditional entry using
    * passed parameter <code>nr</code>.
    * @param nr number of row for conditional entry
    */
    protected PropertyValue[] Conditions(int nr) {
        PropertyValue[] con = new PropertyValue[5];
        CellAddress ca = new CellAddress();
        ca.Column = 1;
        ca.Row = 5;
        ca.Sheet = 0;
        con[0] = new PropertyValue();
        con[0].Name = "StyleName";
        con[0].Value = "Result2";
        con[1] = new PropertyValue();
        con[1].Name = "Formula1";
        con[1].Value = "$Sheet1.$B$"+nr;
        con[2] = new PropertyValue();
        con[2].Name = "Formula2";
        con[2].Value = "";
        con[3] = new PropertyValue();
        con[3].Name = "Operator";
        con[3].Value = ConditionOperator.EQUAL;
        con[4] = new PropertyValue();
        con[4].Name = "SourcePosition";
        con[4].Value = ca;
        return con;
    }

    /**
    * Forces object environment recreation.
    */
    protected void after() {
        this.disposeEnvironment();
    }
}  // finish class _XSheetConditionalEntries


