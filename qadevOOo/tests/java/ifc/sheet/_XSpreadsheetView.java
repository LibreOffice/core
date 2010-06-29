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
import lib.Status;
import lib.StatusException;

import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetView;

/**
* Testing <code>com.sun.star.sheet.XSpreadsheetView</code>
* interface methods :
* <ul>
*  <li><code> getActiveSheet()</code></li>
*  <li><code> setActiveSheet()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Sheet'</code> (of type <code>XSpreadsheet</code>):
*   to set new active spreadsheet </li>
* <ul> <p>
* @see com.sun.star.sheet.XSpreadsheetView
*/
public class _XSpreadsheetView extends MultiMethodTest {

    public XSpreadsheetView oObj = null;
    public XSpreadsheet oSheet = null;

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getActiveSheet() {
        oSheet = oObj.getActiveSheet();
        tRes.tested("getActiveSheet()", oSheet != null);
    }

    /**
    * Test sets new active sheet that was obtained by relation
    * <code>'Sheet'</code>, gets the current active sheet and compares
    * returned value with value that was stored by method
    * <code>getFilterFields()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getActiveSheet() </code> : to have the current
    *  active sheet </li>
    * </ul>
    */
    public void _setActiveSheet() {
        requiredMethod("getActiveSheet()");

        XSpreadsheet new_Sheet = (XSpreadsheet)tEnv.getObjRelation("Sheet");
        if (new_Sheet == null) throw new StatusException(Status.failed
            ("Relation 'Sheet' not found"));

        oObj.setActiveSheet(new_Sheet);
        new_Sheet = oObj.getActiveSheet();
        tRes.tested("setActiveSheet()", !oSheet.equals(new_Sheet));
    }


}  // finish class _XSpreadsheetView


