/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


