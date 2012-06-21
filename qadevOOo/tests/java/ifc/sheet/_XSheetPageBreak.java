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

