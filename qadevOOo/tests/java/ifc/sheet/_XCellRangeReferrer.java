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

import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XCellRangeReferrer;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sheet.XCellRangeReferrer</code>
* interface methods :
* <ul>
*  <li><code> getReferredCells() </code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DATAAREA'</code> (of type <code>CellRangeAddress</code>):
*   to have cell range address </li>
* <ul> <p>
* @see com.sun.star.sheet.XCellRangeReferrer
* @see com.sun.star.table.CellRangeAddress
*/
public class _XCellRangeReferrer extends MultiMethodTest {

    public XCellRangeReferrer oObj = null;

    /**
    * Test calls the method and compares cell range address of returned cell range
    * with cell range address gotten by relation <code>'DATAAREA'</code>. <p>
    * Has <b> OK </b> status if all fields of cell range addresses are equal. <p>
    */
    public void _getReferredCells() {
        boolean bResult = true;

        CellRangeAddress oldCRA = (CellRangeAddress)
                                    tEnv.getObjRelation("DATAAREA");
        if (oldCRA == null) throw new StatusException(Status.failed
            ("Relation 'DATAAREA' not found"));

        XCellRange cr = oObj.getReferredCells();
        log.println(cr.toString());

        if (cr == null) {
            log.println("getReferredCells returned NULL.");
            tRes.tested("getReferredCells()", false);
            return;
        }

        XCellRangeAddressable xCRA = (XCellRangeAddressable)
                UnoRuntime.queryInterface(XCellRangeAddressable.class, cr);

        CellRangeAddress objCRA = xCRA.getRangeAddress();

        log.println("The named range was 'ANamedRange A1:B2'");
        log.println("Expected CellRangeAddress: (" + oldCRA.Sheet +
            "," + oldCRA.StartColumn + "," + oldCRA.StartRow + "," +
            oldCRA.EndColumn + "," + objCRA.EndRow + ")");
        log.println("CellRangeAddress gotten: (" + objCRA.Sheet + ","
            + objCRA.StartColumn + "," + objCRA.StartRow + "," +
            objCRA.EndColumn + "," + objCRA.EndRow + ")");

        bResult &= objCRA.EndColumn   == oldCRA.EndColumn;
        bResult &= objCRA.EndRow       == oldCRA.EndRow;
        bResult &= objCRA.Sheet       == oldCRA.Sheet;
        bResult &= objCRA.StartColumn == oldCRA.StartColumn;
        bResult &= objCRA.StartRow       == oldCRA.StartRow;

        tRes.tested("getReferredCells()", bResult);
    }
}


