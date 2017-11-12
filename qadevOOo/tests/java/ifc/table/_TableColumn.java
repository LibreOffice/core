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

package ifc.table;

import lib.MultiPropertyTest;
import util.ValueComparer;

import com.sun.star.table.XCellRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.table.TableColumn</code>
* service properties :
* <ul>
*  <li><code> Width</code></li>
*  <li><code> OptimalWidth</code></li>
*  <li><code> IsVisible</code></li>
*  <li><code> IsStartOfNewPage</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.table.TableColumn
*/
public class _TableColumn extends MultiPropertyTest {

    public void _OptimalWidth() {
        boolean res = false;
        try {
            XInterface tObject = tEnv.getTestObject();
            XCellRange aRange = UnoRuntime.queryInterface(XCellRange.class, tObject);
            aRange.getCellByPosition(0,0).setFormula("That's a pretty long text");
            Object width_before = oObj.getPropertyValue("Width");
            log.println("Before setting to optimal width: "+width_before);
            oObj.setPropertyValue("OptimalWidth", Boolean.TRUE);
            Object width_after = oObj.getPropertyValue("Width");
            log.println("After setting to optimal width: "+width_after);
            res = ! ValueComparer.equalValue(width_before,width_after);
        } catch (Exception e) {
           res = false;
           e.printStackTrace(log);
        }

        tRes.tested("OptimalWidth",res );
    }

} //finish class _TableColumn


