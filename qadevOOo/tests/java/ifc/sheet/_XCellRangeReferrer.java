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

        if (cr == null) {
            log.println("getReferredCells returned NULL.");
            tRes.tested("getReferredCells()", false);
            return;
        }
        log.println(cr.toString());

        XCellRangeAddressable xCRA = UnoRuntime.queryInterface(XCellRangeAddressable.class, cr);

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


