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

        XSheetCellRange oSheetRange = (XSheetCellRange)
            UnoRuntime.queryInterface(XSheetCellRange.class, oRange) ;
        log.println("getting Cursor");
        XSheetCellCursor oCursor = oObj.createCursorByRange(oSheetRange);
        tRes.tested("createCursorByRange()", oCursor != null);
    }

} // finisch class _XSpreadsheet


