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

import com.sun.star.sheet.TablePageBreakData;
import com.sun.star.sheet.XSheetPageBreak;

/**
* Testing <code>com.sun.star.sheet.XSheetPageBreak</code>
* interface methods :
* <ul>
*  <li><code> getColumnPageBreaks()</code></li>
*  <li><code> getRowPageBreaks()</code></li>
*  <li><code> removeAllManualPageBreaks()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetPageBreak
*/
public class _XSheetPageBreak extends MultiMethodTest {

    public XSheetPageBreak oObj = null;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getColumnPageBreaks() {
        TablePageBreakData[] breakArr = oObj.getColumnPageBreaks();
        tRes.tested("getColumnPageBreaks()", breakArr != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getRowPageBreaks() {
        TablePageBreakData[] breakArr = oObj.getRowPageBreaks();
        tRes.tested("getRowPageBreaks()", breakArr != null);
    }

    /**
    * Test calls the method and checks that all breaks gone. <p>
    * Has <b> OK </b> status if number of column breaks and number of row breaks
    * are equal to zero. <p>
    */
    public void _removeAllManualPageBreaks() {
        oObj.removeAllManualPageBreaks();

        int manualRowPageBreaks = 0;
        int manualColumnPageBreaks = 0;

        log.println("Check if all breaks are gone");

        TablePageBreakData[] breakArr = oObj.getColumnPageBreaks();
        for (int i = 0; i < breakArr.length; i++)
            if (breakArr[i].ManualBreak) manualColumnPageBreaks++;

        breakArr = oObj.getRowPageBreaks();
        for (int i = 0; i < breakArr.length; i++)
            if (breakArr[i].ManualBreak) manualRowPageBreaks++ ;

        tRes.tested(
            "removeAllManualPageBreaks()",
            manualColumnPageBreaks == 0 && manualRowPageBreaks == 0);
    }
}

