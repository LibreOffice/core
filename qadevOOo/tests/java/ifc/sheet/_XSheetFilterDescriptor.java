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

import com.sun.star.sheet.TableFilterField;
import com.sun.star.sheet.XSheetFilterDescriptor;

/**
* Testing <code>com.sun.star.sheet.XSheetFilterDescriptor</code>
* interface methods :
* <ul>
*  <li><code> getFilterFields()</code></li>
*  <li><code> setFilterFields()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetFilterDescriptor
*/
public class _XSheetFilterDescriptor extends MultiMethodTest {

    public XSheetFilterDescriptor oObj = null;
    public TableFilterField[] TFF = null;

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getFilterFields(){
        TFF = oObj.getFilterFields();
        tRes.tested("getFilterFields()", TFF != null );
    }

    /**
    * Test sets new value of the definitions of the filter fields,
    * gets the current definitions of the filter fields and compares
    * returned value with value that was stored by method
    * <code>getFilterFields()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getFilterFields() </code> : to have the current definitions
    *  of the filter fields </li>
    * </ul>
    */
    public void _setFilterFields() {
        requiredMethod("getFilterFields()");
        TableFilterField[] newTFF = new TableFilterField[1];
        newTFF[0] = new TableFilterField();
        oObj.setFilterFields(newTFF);
        newTFF = oObj.getFilterFields();
        tRes.tested("setFilterFields()", ! ValueComparer.equalValue(TFF, newTFF));
    }

}  // finish class _XSheetFilterDescriptor


