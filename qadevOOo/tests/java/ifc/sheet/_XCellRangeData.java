/*************************************************************************
 *
 *  $RCSfile: _XCellRangeData.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-11-18 16:24:14 $
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

