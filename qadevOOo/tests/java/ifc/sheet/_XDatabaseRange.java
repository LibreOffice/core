/*************************************************************************
 *
 *  $RCSfile: _XDatabaseRange.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:00:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

        for(int i = STARTROW; i < ENDROW; i++) {
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

    final short COL = 0;
    final short STARTROW = 0;
    final short ENDROW = 5;

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

    protected void after() {
        disposeEnvironment();
    }
}


