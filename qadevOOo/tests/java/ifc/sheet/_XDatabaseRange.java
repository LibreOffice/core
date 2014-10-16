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

import com.sun.star.beans.PropertyValue;
import com.sun.star.sheet.SubTotalColumn;
import com.sun.star.sheet.XDatabaseRange;
import com.sun.star.sheet.XSheetFilterDescriptor;
import com.sun.star.sheet.XSubTotalDescriptor;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;

/**
* Testing <code>com.sun.star.sheet.XDatabaseRange</code>
* interface methods :
* <ul>
*  <li><code> getDataArea()</code></li>
*  <li><code> setDataArea()</code></li>
*  <li><code> getSortDescriptor()</code></li>
*  <li><code> getFilterDescriptor()</code></li>
*  <li><code> getSubTotalDescriptor()</code></li>
*  <li><code> getImportDescriptor()</code></li>
*  <li><code> refresh()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DATAAREA'</code> (of type <code>CellRangeAddress</code>):
*   to have cell range address for test of method <code>getDataArea()</code></li>
*   <li> <code>'XCELLRANGE'</code> (of type <code>XCellRange</code>):
*   cell range of the spreadsheet with database range,
*   to get values of cell</li>
* <ul> <p>
* @see com.sun.star.sheet.XDatabaseRange
* @see com.sun.star.table.CellRangeAddress
*/
public class _XDatabaseRange extends MultiMethodTest {

    public XDatabaseRange oObj = null;
    CellRangeAddress oldCRA = null;
    XCellRange xCellRange = null;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    @Override
    protected void before() {
        oldCRA = (CellRangeAddress)tEnv.getObjRelation("DATAAREA");
        if (oldCRA == null) {
            throw new StatusException(Status.failed
                ("Relation 'DATAAREA' not found"));
        }
        xCellRange = (XCellRange)tEnv.getObjRelation("XCELLRANGE");
        if (xCellRange == null) {
            throw new StatusException(Status.failed
                ("Relation 'XCELLRANGE' not found"));
        }
    }

    /**
    * Test calls the method and compares returned cell range address with
    * cell range address obtained by object relation <code>'DATAAREA'</code>.<p>
    * Has <b> OK </b> status if all fields of cell range addresses are equal. <p>
    */
    public void _getDataArea() {
        boolean bResult = true;
        CellRangeAddress objCRA = oObj.getDataArea();
        bResult &= objCRA.EndColumn   == oldCRA.EndColumn;
        bResult &= objCRA.EndRow       == oldCRA.EndRow;
        bResult &= objCRA.Sheet       == oldCRA.Sheet;
        bResult &= objCRA.StartColumn == oldCRA.StartColumn;
        bResult &= objCRA.StartRow       == oldCRA.StartRow;
        tRes.tested("getDataArea()", bResult);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getFilterDescriptor() {
        XSheetFilterDescriptor FD = oObj.getFilterDescriptor();
        tRes.tested("getFilterDescriptor()", FD != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getImportDescriptor() {
        PropertyValue[] pva = oObj.getImportDescriptor();
        tRes.tested("getImportDescriptor()", pva != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getSortDescriptor() {
        PropertyValue[] pva = oObj.getSortDescriptor();
        tRes.tested("getSortDescriptor()", pva != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getSubTotalDescriptor() {
        STD = oObj.getSubTotalDescriptor();
        tRes.tested("getSubTotalDescriptor()", STD != null);
    }

    XSubTotalDescriptor STD = null;

    /**
    * Test adds new SubTotalDescriptor and checks value of cell with
    * subtotal sum after refresh() call. <p>
    * Has <b> OK </b> if value of cell with subtotal sum was changed
    * after refresh() call.<p>
    */
    public void _refresh() {
        requiredMethod("getSubTotalDescriptor()");
        requiredMethod("setDataArea()");

        for(int i = STARTROW; i < ENDROW+1; i++) {
            try {
                XCell cell = xCellRange.getCellByPosition(COL, i);
                cell.setValue(i);
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected exception");
                e.printStackTrace(log);
                tRes.tested("refresh()", false);
            }
        }

        SubTotalColumn[] STC = new SubTotalColumn[1];
        STC[0] = new SubTotalColumn();
        STC[0].Column = COL;
        STC[0].Function = com.sun.star.sheet.GeneralFunction.SUM;

        double oldVal = 0;
        try {
            XCell checkCell = xCellRange.getCellByPosition(COL, ENDROW);
            oldVal = checkCell.getValue();
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            tRes.tested("refresh()", false);
        }
        log.println("Value of the cell (" + COL + ", " + ENDROW +
            ") : " + oldVal );

        log.println("Set new SubTotal descriptor...");

        STD.clear();
        STD.addNew(STC, 1);

        double valBeforeRefresh = 0;
        try {
            XCell checkCell = xCellRange.getCellByPosition(COL, ENDROW);
            valBeforeRefresh = checkCell.getValue();
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            tRes.tested("refresh()", false);
        }
        log.println("Value of the cell (" + COL + ", " + ENDROW +
            ") : " + valBeforeRefresh );

        log.println("Now call refresh()...");
        oObj.refresh();

        double valAfterRefresh = 0;
        try {
            XCell checkCell = xCellRange.getCellByPosition(COL, ENDROW);
            valAfterRefresh = checkCell.getValue();
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            tRes.tested("refresh()", false);
        }
        log.println("Value of the cell (" + COL + ", " + ENDROW +
            ") : " + valAfterRefresh );

        tRes.tested("refresh()", oldVal != valAfterRefresh &&
            oldVal == valBeforeRefresh);
    }

    static final short COL = 0;
    static final short STARTROW = 0;
    static final short ENDROW = 5;

    /**
    * Test creates new cell range address and calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setDataArea() {
        executeMethod("getDataArea()");
        CellRangeAddress newCRA = new CellRangeAddress();
        newCRA.Sheet = oldCRA.Sheet;
        newCRA.StartColumn = COL;
        newCRA.EndColumn = COL;
        newCRA.StartRow = STARTROW;
        newCRA.EndRow = ENDROW;

        oObj.setDataArea(newCRA);

        tRes.tested("setDataArea()", true);
    }

    @Override
    protected void after() {
        disposeEnvironment();
    }
}


