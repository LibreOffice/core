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

import com.sun.star.sheet.XSheetCellRange;
import com.sun.star.sheet.XSpreadsheet;

/**
* Testing <code>com.sun.star.sheet.XSheetCellRange</code>
* interface methods :
* <ul>
*  <li><code> getSpreadsheet()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetCellRange
*/
public class _XSheetCellRange extends MultiMethodTest {

    public XSheetCellRange oObj = null;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getSpreadsheet(){
        XSpreadsheet oSheet = oObj.getSpreadsheet();
        tRes.tested("getSpreadsheet()", oSheet != null);
    }

} // EOC _XSheetCellRange

