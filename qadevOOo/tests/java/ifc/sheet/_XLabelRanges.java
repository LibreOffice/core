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

import com.sun.star.sheet.XLabelRanges;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XLabelRanges</code>
* interface methods :
* <ul>
*  <li><code> addNew()</code></li>
*  <li><code> removeByIndex()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XLabelRanges
*/
public class _XLabelRanges extends MultiMethodTest {

    public XLabelRanges oObj = null;

    /**
    * Test calls the method and compares number of label range before method
    * call and after. <p>
    * Has <b>OK</b> status if number of label range before method call is less
    * than after and no exceptions were thrown. <p>
    */
    public void _addNew() {
        int anz = oObj.getCount();
        oObj.addNew(
            new CellRangeAddress((short)0, 1, 0, 1, 0),
            new CellRangeAddress((short)0, 1, 1, 1, 6) );
        tRes.tested("addNew()", anz < oObj.getCount());
    }

    /**
    * Test removes an existent label range first and tries to remove
    * non-existent label range. <p>
    * Has <b> OK </b> status if number of range decreased by one after first
    * call and exception was thrown in second. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addNew() </code> : to have one label range at least </li>
    * </ul>
    */
    public void _removeByIndex() {
        requiredMethod("addNew()");
        int anz = oObj.getCount();
        log.println("First remove an existent LabelRange");

        oObj.removeByIndex(anz - 1);
        boolean result = (anz - 1 == oObj.getCount());

        log.println("Remove a nonexistent LabelRange");
        try {
            oObj.removeByIndex(anz);
            log.println("No Exception thrown while removing nonexisting "+
                "LabelRange");
            result &= false;
        } catch (com.sun.star.uno.RuntimeException e) {
            log.println("Expected exception thrown while removing "+
                "nonexisting LabelRange: "+e);
            result &= true;
        }

        tRes.tested("removeByIndex()", result);
    }

}  // finish class _XLabelRanges


