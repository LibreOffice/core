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


