/*************************************************************************
 *
 *  $RCSfile: _XMultipleOperation.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:01:55 $
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

import com.sun.star.awt.Point;
import com.sun.star.sheet.TableOperationMode;
import com.sun.star.sheet.XCellAddressable;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XMultipleOperation;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.UnoRuntime;


public class _XMultipleOperation extends MultiMethodTest {
    public XMultipleOperation oObj = null;
    protected XSpreadsheet oSheet = null;

    protected void before() {
        oSheet = (XSpreadsheet) tEnv.getObjRelation("SHEET");

        if (oSheet == null) {
            log.println("Object relation oSheet is missing");
            log.println("Trying to query the needed Interface");
            oSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                             XSpreadsheet.class, tEnv.getTestObject());

            if (oSheet == null) {
                throw new StatusException(Status.failed(
                                                  "Object relation oSheet is missing"));
            } else {
                log.println("We have a sheet and restrict the range to A1:D4");
                oObj = (XMultipleOperation) UnoRuntime.queryInterface(
                               XMultipleOperation.class,
                               oSheet.getCellRangeByName("$A$1:$D$4"));
            }
        }
    }

    public void _setTableOperation() {
        boolean res = true;
        XCellRange cellRange = oSheet.getCellRangeByName("$A$17:$A$17");
        XCellRangeAddressable CRA = (XCellRangeAddressable) UnoRuntime.queryInterface(
                                            XCellRangeAddressable.class,
                                            cellRange);
        XCell cell = null;
        XCell cell2 = null;

        try {
            cell = oSheet.getCellByPosition(0, 16);
            cell.setFormula("=a15+a16");
            cell = oSheet.getCellByPosition(0, 14);
            cell2 = oSheet.getCellByPosition(0, 15);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception while getting Cell " + e.getMessage());
        }

        XCellAddressable CA = (XCellAddressable) UnoRuntime.queryInterface(
                                      XCellAddressable.class, cell);
        XCellAddressable CA2 = (XCellAddressable) UnoRuntime.queryInterface(
                                       XCellAddressable.class, cell2);
        Point[] cellCoords = new Point[3];
        double[] cellValues = new double[3];

        log.println("filling cells");
        fillCells();
        log.println("setting TableOperation with parameter ROW");
        oObj.setTableOperation(CRA.getRangeAddress(), TableOperationMode.ROW,
                               CA.getCellAddress(), CA2.getCellAddress());
        log.println("checking values");
        cellCoords = new Point[] {
            new Point(1, 1), new Point(2, 1), new Point(3, 1)
        };
        cellValues = new double[] { 5, 10, 15 };
        res &= checkValues(cellCoords, cellValues);

        log.println("filling cells");
        fillCells();
        log.println("setting TableOperation with parameter COLUMN");
        oObj.setTableOperation(CRA.getRangeAddress(),
                               TableOperationMode.COLUMN, CA.getCellAddress(),
                               CA2.getCellAddress());
        log.println("checking values");
        cellCoords = new Point[] {
            new Point(1, 1), new Point(1, 2), new Point(1, 3)
        };
        cellValues = new double[] { 12, 24, 36 };
        res &= checkValues(cellCoords, cellValues);

        log.println("filling cells");
        fillCells();
        log.println("setting TableOperation with parameter BOTH");
        oObj.setTableOperation(CRA.getRangeAddress(), TableOperationMode.BOTH,
                               CA.getCellAddress(), CA2.getCellAddress());
        log.println("checking values");
        cellCoords = new Point[] {
            new Point(1, 1), new Point(2, 2), new Point(3, 3)
        };
        cellValues = new double[] { 17, 34, 51 };
        res &= checkValues(cellCoords, cellValues);

        tRes.tested("setTableOperation()", res);
    }

    protected void fillCells() {
        XCell cell = null;

        try {
            for (int k = 1; k < 5; k++) {
                cell = oSheet.getCellByPosition(0, k);
                cell.setValue(k * 12);
                cell = oSheet.getCellByPosition(k, 0);
                cell.setValue(k * 5);
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception while filling Cells " + e.getMessage());
        }
    }

    protected boolean checkValues(Point[] cellCoords, double[] cellValues) {
        boolean res = true;

        for (int i = 0; i < cellValues.length; i++) {
            try {
                boolean locres = oSheet.getCellByPosition(cellCoords[i].X,
                                                          cellCoords[i].Y)
                                       .getValue() == cellValues[i];
                res &= locres;

                if (!locres) {
                    log.println("Result differs for cell (" +
                                cellCoords[i].X + "," + cellCoords[i].Y +
                                ")");
                    log.println("Expected: " + cellValues[i]);
                    log.println("Getting: " +
                                oSheet.getCellByPosition(cellCoords[i].X,
                                                         cellCoords[i].Y)
                                      .getValue());
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Exception while checking Values " +
                            e.getMessage());
                res &= false;
            }
        }

        return res;
    }

    /**
     * Restores initial component text.
     */
    protected void after() {
        disposeEnvironment();
    }
}