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

package ifc.table;

import lib.MultiMethodTest;

import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XSheetCellCursor;
import com.sun.star.sheet.XSheetCellRange;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCellCursor;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.table.XCellCursor</code>
* interface methods :
* <ul>
*  <li><code> gotoStart()</code></li>
*  <li><code> gotoEnd()</code></li>
*  <li><code> gotoNext()</code></li>
*  <li><code> gotoPrevious()</code></li>
*  <li><code> gotoOffset()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'SHEET'</code> (of type <code>XSpreadsheet</code>):
*   is used for creating a new cell range.</li>
* <ul> <p>
*
* The component tested <b>must implement</b>
* <code>XCellRangeAddressable</code> interface. <p>
*
* Test is <b> NOT </b> multithread compliant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.table.XCellCursor
*/
public class _XCellCursor extends MultiMethodTest {

    public XCellCursor oObj = null;

    /**
     * A new cell range is created using spreadsheet passed by relation.
     * The method is tested on that range. <code>gotoEnd</code> is
     * called and range address is checked.<p>
     * Has <b> OK </b> status if cursor was collapsed to a single
     * cell (i.e. start column is the same as end column) after
     * movement. <p>
     */
    public void _gotoEnd(){
        //gotoEnd gets its own cursor to see a change
        XSpreadsheet oSheet = (XSpreadsheet) tEnv.getObjRelation("SHEET");
        XCellRange testRange = oSheet.getCellRangeByName("$A$1:$g$7") ;
        XSheetCellRange testSheetRange = UnoRuntime.queryInterface(XSheetCellRange.class,testRange);
        XSheetCellCursor oCellCursor = oSheet.createCursorByRange
            (testSheetRange);
        XCellCursor oCursor = UnoRuntime.queryInterface(XCellCursor.class,oCellCursor);

        boolean bResult = false;
        int startCol, endCol, startRow, endRow = 0;

        XCellRangeAddressable oRange = UnoRuntime.queryInterface(XCellRangeAddressable.class, oCursor);
        oCursor.gotoEnd();
        CellRangeAddress oAddr = oRange.getRangeAddress();
        startRow = oAddr.StartRow;
        startCol = oAddr.StartColumn;
        endRow = oAddr.EndRow;
        endCol = oAddr.EndColumn;
        if ((startCol == endCol) && (endRow == startRow)){
            bResult = true;
        }

        tRes.tested( "gotoEnd()", bResult );
    }

    /**
    * Forces object environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }


} //EOC _XCellCursor
