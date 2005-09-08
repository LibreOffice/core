/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XCellRangeReferrer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:44:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


