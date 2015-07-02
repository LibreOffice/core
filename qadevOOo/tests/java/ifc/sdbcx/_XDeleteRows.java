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

package ifc.sdbcx;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbcx.XDeleteRows;
import com.sun.star.sdbcx.XRowLocate;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sdbcx.XDeleteRows</code>
* interface methods :
* <ul>
*  <li><code> deleteRows()</code></li>
* </ul> <p>
* @see com.sun.star.sdbcx.XDeleteRows
*/
public class _XDeleteRows extends MultiMethodTest {
    // oObj filled by MultiMethodTest
    public XDeleteRows oObj = null ;

    /**
    * Retrieves bookmark using XRowLocate and deletes
    * row pointed by this bookmark. <p>
    * Has OK status if number of rows after deleting is less than before
    * and no exception rises while method call, FAILED otherwise. <p>
    */
    public void _deleteRows() {
        XRowLocate xRowLocate = UnoRuntime.queryInterface(XRowLocate.class, oObj);
        XResultSet xResultSet = UnoRuntime.queryInterface(XResultSet.class, oObj);
        if (xRowLocate == null || xResultSet == null) {
            log.println("The test must be modified according to "+
                "component testcase");
            throw new StatusException(Status.failed(
                "The component doesn't support one of the "+
                    "required interfaces"));
        }

        int rowsBefore = 0, rowsAfter = 0;
        Object bkmrk = null;
        try {
            xResultSet.last();
            rowsBefore = xResultSet.getRow();
            xResultSet.first();
            bkmrk = xRowLocate.getBookmark();
            oObj.deleteRows(new Object[] {bkmrk});
            xResultSet.last();
            rowsAfter = xResultSet.getRow();
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("SQLException:" + e);
            tRes.tested("deleteRows()", false);
            return;
        }

        log.println("Rows before: " + rowsBefore + ", after: " + rowsAfter);
        tRes.tested("deleteRows()", rowsBefore - 1 == rowsAfter);
    }

    @Override
    protected void after() {
        disposeEnvironment();
    }
}  // finish class _XDeleteRows

