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
                emptyData[i][j] = "";
            }
        }
        oObj.setDataArray(emptyData);
        tRes.tested("setDataArray()", res);
    }
}

