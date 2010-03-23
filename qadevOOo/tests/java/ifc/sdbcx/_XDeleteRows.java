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
    * and no exception rizes while method call, FAILED otherwise. <p>
    */
    public void _deleteRows() {
        XRowLocate xRowLocate = (XRowLocate)
            UnoRuntime.queryInterface(XRowLocate.class, oObj);
        XResultSet xResultSet = (XResultSet)
            UnoRuntime.queryInterface(XResultSet.class, oObj);
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

    protected void after() {
        disposeEnvironment();
    }
}  // finish class _XDeleteRows

