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

import com.sun.star.sheet.XSheetCellCursor;
import com.sun.star.sheet.XSheetCellRange;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sheet.XSpreadsheet</code>
* interface methods :
* <ul>
*  <li><code> createCursor()</code></li>
*  <li><code> createCursorByRange()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSpreadsheet
*/
public class _XSpreadsheet extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XSpreadsheet oObj = null;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _createCursor() {
        log.println("Testing createCursor");
        XSheetCellCursor oCursor = oObj.createCursor() ;
        tRes.tested("createCursor()", oCursor != null);
    }

    /**
    * Test gets a cell range, call method using this cell range
    * and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null
    * and no exceptions were thrown. <p>
    */
    public void _createCursorByRange() {
        log.println("Testing createCursorByRange");

        log.println("getting cellrange");
        XCellRange oRange = null;
        try {
            oRange = oObj.getCellRangeByPosition (1, 1, 2, 3);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Can't get cell range by address");
            e.printStackTrace(log);
            tRes.tested("createCursorByRange()", false);
        }

        XSheetCellRange oSheetRange = UnoRuntime.queryInterface(XSheetCellRange.class, oRange) ;
        log.println("getting Cursor");
        XSheetCellCursor oCursor = oObj.createCursorByRange(oSheetRange);
        tRes.tested("createCursorByRange()", oCursor != null);
    }

} // finisch class _XSpreadsheet


