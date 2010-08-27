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
            XCellRange aRange = (XCellRange)
                    UnoRuntime.queryInterface(XCellRange.class, tObject);
            aRange.getCellByPosition(0,0).setFormula("Thats a pretty long text");
            Object width_before = oObj.getPropertyValue("Width");
            log.println("Before setting to optimal width: "+width_before);
            oObj.setPropertyValue("OptimalWidth", new Boolean(true));
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


