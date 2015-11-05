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

import com.sun.star.sheet.CellDeleteMode;
import com.sun.star.sheet.CellInsertMode;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XCellRangeMovement;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableRows;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sheet.XCellRangeMovement</code>
* interface methods :
* <ul>
*  <li><code> insertCells()</code></li>
*  <li><code> removeRange()</code></li>
*  <li><code> moveRange()</code></li>
*  <li><code> copyRange()</code></li>
* </ul> <p>
* Test object must implements interfaces <code>XCellRangeAddressable</code>
* and <code>XSpreadsheet</code> also. <p>
* @see com.sun.star.sheet.XCellRangeMovement
* @see com.sun.star.sheet.XSpreadsheet
* @see com.sun.star.sheet.XCellRangeAddressable
*/
public class _XCellRangeMovement extends MultiMethodTest {

    public XCellRangeMovement oObj = null;

    /**
    * Test sets specific values to cells in the range, copies this cell range
    * to another position in document and checks cell's values in new position.<p>
    * Has <b> OK </b> status if cell's values in source range are equal to
    * cell's values in destination range and no exceptions were thrown. <p>
    */
    public void _copyRange(){
        log.println("Prepare cells before test methods.");
        XSpreadsheet oSheet = UnoRuntime.queryInterface(XSpreadsheet.class, oObj);
        try {
            oSheet.getCellByPosition(1,1).setValue(100);
            oSheet.getCellByPosition(1,2).setValue(200);
            oSheet.getCellByPosition(2,1).setValue(300);
            oSheet.getCellByPosition(2,2).setValue(400);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            tRes.tested("copyRange()", false);
        }

        XCellRangeAddressable oAddr =
            UnoRuntime.queryInterface (XCellRangeAddressable.class, oObj);
        short iSheet = oAddr.getRangeAddress().Sheet;
        CellAddress sDest;
        CellRangeAddress sSrc;

        sSrc = new CellRangeAddress(iSheet, 1, 1, 2, 2);
        sDest = new CellAddress(iSheet, 1, 10);
        boolean result = true;
        boolean loc_result = true;

        oObj.copyRange(sDest, sSrc);
        try {
            loc_result  = (oSheet.getCellByPosition(1, 10).getValue() == 100);
            loc_result &= (oSheet.getCellByPosition(1, 11).getValue() == 200);
            loc_result &= (oSheet.getCellByPosition(2, 10).getValue() == 300);
            loc_result &= (oSheet.getCellByPosition(2, 11).getValue() == 400);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            tRes.tested("copyRange()", false);
        }

        result &= loc_result;
        tRes.tested("copyRange()", result);
    }

    /**
    * Test sets specific values to cells in the two contiguous rows, inserts
    * new empty row between them and checks value in
    * one cell of the inserted row. <p>
    * Has <b> OK </b> status if value of cell in the inserted row is zero
    * and no exceptions were thrown. <p>
    */
    public void _insertCells(){
        boolean result = false;

        XSpreadsheet oSheet = UnoRuntime.queryInterface(XSpreadsheet.class, oObj);
        XCellRangeAddressable oAddr = UnoRuntime.queryInterface (XCellRangeAddressable.class, oObj);
        short iSheet = oAddr.getRangeAddress().Sheet;
        try {
            oSheet.getCellByPosition(0,20).setValue(100);
            oSheet.getCellByPosition(1,20).setValue(100);
            oSheet.getCellByPosition(2,20).setValue(100);
            oSheet.getCellByPosition(3,20).setValue(100);
            oSheet.getCellByPosition(0,21).setValue(200);
            oSheet.getCellByPosition(1,21).setValue(200);
            oSheet.getCellByPosition(2,21).setValue(200);
            oSheet.getCellByPosition(3,21).setValue(200);

            // catch some sleight of hand threads
            if (oSheet.getCellByPosition(1,21).getValue() != 200){
                log.println("Cells were already inserted. "+
                    "Delete old cells now");
                XColumnRowRange oColumnRowRange = UnoRuntime.queryInterface(XColumnRowRange.class, oSheet);

                XTableRows oRows = oColumnRowRange.getRows();
                oRows.removeByIndex(21,1);
            }
            CellRangeAddress sSrc = new CellRangeAddress(iSheet, 0, 21, 5, 21);
            oObj.insertCells (sSrc, CellInsertMode.DOWN) ;

            // check the result
            double res = oSheet.getCellByPosition(1, 21).getValue();
            if (res == 0.0) {
                result = true;
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("insertCells()", result);
    }

    /**
    * Test sets specific values to cells in the range, moves this cell range
    * to another position in document and checks cell's values in new position.
    * <p>Has <b>OK</b> status if sum of values in source range is equal to sum
    * of values in destination range and no exceptions were thrown. <p>
    */
    public void _moveRange(){
        boolean result = false;

        XSpreadsheet oSheet = UnoRuntime.queryInterface(XSpreadsheet.class, oObj);

        XCellRangeAddressable oAddr = UnoRuntime.queryInterface (XCellRangeAddressable.class, oObj);

        short iSheet = oAddr.getRangeAddress().Sheet;
        //prepare source range
        try {
            oSheet.getCellByPosition(4,0).setValue(111);
            oSheet.getCellByPosition(4,1).setValue(222);

            CellRangeAddress sSrc = new CellRangeAddress(iSheet, 4, 0, 4, 1);
            CellAddress sDest = new CellAddress(iSheet, 4, 4);
            oObj.moveRange(sDest, sSrc);

            double cntA = 0;
            double cntB = 0;
            cntA = oSheet.getCellByPosition(4, 4).getValue();
            cntB = oSheet.getCellByPosition(4, 5).getValue();
            if (cntA + cntB == 333.0){ result = true; }
            //clean up
            oSheet.getCellByPosition(4,4).setValue(0);
            oSheet.getCellByPosition(4,5).setValue(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("moveRange()", result);
    }


    /**
    * Test sets specific values to cells in the range, deletes this cell range
    * from document and checks values of cells in position
    * of the deleted range.
    * <p>Has <b>OK</b> status if sum of cell values in position of the deleted
    * range is equal to zero and no exceptions were thrown. <p>
    */
    public void _removeRange(){
        boolean result = false;

        XSpreadsheet oSheet = UnoRuntime.queryInterface(XSpreadsheet.class, oObj);
        XCellRangeAddressable oAddr = UnoRuntime.queryInterface (XCellRangeAddressable.class, oObj);
        short iSheet = oAddr.getRangeAddress().Sheet;
        try {
            //prepare source range
            oSheet.getCellByPosition(5, 0).setValue(333);
            oSheet.getCellByPosition(5, 1).setValue(444);

            CellRangeAddress sSrc = new CellRangeAddress(iSheet, 5, 0, 5, 1);
            oObj.removeRange(sSrc, CellDeleteMode.UP);

            double cntA = 0;
            double cntB = 0;
            cntA = oSheet.getCellByPosition(5, 0).getValue();
            cntB = oSheet.getCellByPosition(5, 1).getValue();
            if (cntA + cntB == 0.0){ result = true; }

            //clean up
            oSheet.getCellByPosition(5, 0).setValue(0);
            oSheet.getCellByPosition(5, 1).setValue(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("removeRange()", result);
    }

} // EOC _XCellRangeMovement

