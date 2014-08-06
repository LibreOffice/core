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

    @Override
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

