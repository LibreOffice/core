/*************************************************************************
 *
 *  $RCSfile: _XCellRangeReferrer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:59:18 $
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


