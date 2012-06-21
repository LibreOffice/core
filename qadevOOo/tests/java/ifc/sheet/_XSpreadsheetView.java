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


