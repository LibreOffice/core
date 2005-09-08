/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XCellRangeData.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:43:16 $
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

