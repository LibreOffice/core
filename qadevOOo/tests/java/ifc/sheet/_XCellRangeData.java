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
import util.ValueComparer;

import com.sun.star.sheet.XCellRangeData;

public class _XCellRangeData extends MultiMethodTest {

    public XCellRangeData oObj = null;
    private Object[][] maCRData = null;

    /**
    * Test calls the method
    * state is OK if the resulting Object array
    * isn't empty
    */
    public void _getDataArray() {
        maCRData = oObj.getDataArray();
        boolean bResult = (maCRData.length > 0);
        tRes.tested("getDataArray()", bResult);
    }

    /**
    * Test creates an Array and calls the method
    * with this Array as argument
    * Then the method getDataArray is called
    * and the resulting Array is compared with the
    * one formerly set.
    */
    public void _setDataArray() {
        Object[][] newData = (Object[][]) tEnv.getObjRelation("NewData");
        if (newData == null) {
            newData = new Object[maCRData.length][maCRData[0].length];
            for (int i=0; i<newData.length; i++) {
                for (int j=0; j<newData[i].length; j++) {
                    newData[i][j] = new Double(10*i +j);
                }
            }
        }
        oObj.setDataArray(newData);
        Object[][] oCRData = oObj.getDataArray();
        boolean res = ValueComparer.equalValue(oCRData[0][0],newData[0][0]);
        res &= ValueComparer.equalValue(oCRData[0][1],newData[0][1]);
        res &= ValueComparer.equalValue(oCRData[1][0],newData[1][0]);
        res &= ValueComparer.equalValue(oCRData[1][1],newData[1][1]);
        // delete values
        Object[][] emptyData = new Object[newData.length][newData[0].length];
        for (int i=0; i<emptyData.length; i++) {
            for (int j=0; j<emptyData[i].length; j++) {
                emptyData[i][j] = new String();
            }
        }
        oObj.setDataArray(emptyData);
        tRes.tested("setDataArray()", res);
    }
}

