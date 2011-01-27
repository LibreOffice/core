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

import com.sun.star.sheet.XDataPilotTable;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;

/**
* Testing <code>com.sun.star.sheet.XDataPilotTable</code>
* interface methods :
* <ul>
*  <li><code> getOutputRange()</code></li>
*  <li><code> refresh()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'OUTPUTRANGE'</code> (of type <code>CellAddress</code>):
*   to check value returned by method <code>getOutputRange()</code> </li>
*  <li> <code>'CELLFORCHANGE'</code> (of type <code>XCell</code>):
*   to check the method refresh(value of this cell will be changed)</li>
*  <li> <code>'CELLFORCHECK'</code> (of type <code>XCell</code>):
* to check the method refresh (value of this cell must be changed after refresh
* call) </li><ul> <p>
* @see com.sun.star.sheet.XDataPilotTable
* @see com.sun.star.table.CellAddress
*/
public class _XDataPilotTable extends MultiMethodTest {

    public XDataPilotTable oObj = null;
    XCell xCellForChange = null;
    XCell xCellForCheck = null;
    CellAddress OutputRange = null;

    protected void before() {
        xCellForChange = (XCell)tEnv.getObjRelation("CELLFORCHANGE");
        xCellForCheck = (XCell)tEnv.getObjRelation("CELLFORCHECK");
        OutputRange = (CellAddress)tEnv.getObjRelation("OUTPUTRANGE");
        if (xCellForChange == null || OutputRange == null ||
                xCellForCheck == null) {
            throw new StatusException(Status.failed("Relation not found"));
        }
    }
    /**
    * Test calls the method and checks returned value using value obtained by
    * object relation <code>'OUTPUTRANGE'</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    */
    public void _getOutputRange(){
        boolean bResult = true;
        CellRangeAddress objRange = oObj.getOutputRange();
        bResult &= OutputRange.Sheet == objRange.Sheet;
        bResult &= OutputRange.Row == objRange.StartRow;
        bResult &= OutputRange.Column == objRange.StartColumn;
        tRes.tested("getOutputRange()", bResult);
    }

    /**
    * Test sets new value of the cell obtained by object relation
    * 'CELLFORCHANGE', and checks value of the cell obtained by object
    * relation 'CELLFORCHECK'.<p>
    * Has <b>OK</b> status if value of the cell obtained by object relation
    * 'CELLFORCHECK' is changed. <p>
    */
    public void _refresh(){
        xCellForChange.setValue(5);
        double oldData = xCellForCheck.getValue();
        oObj.refresh();
        double newData = xCellForCheck.getValue();
        log.println("Old data:" + oldData + "; new data:" + newData);

        tRes.tested("refresh()", oldData != newData);
    }
}

