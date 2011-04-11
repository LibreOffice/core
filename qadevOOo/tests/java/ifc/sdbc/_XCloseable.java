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

package ifc.sdbc;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sdbc.XCloseable</code>
* interface methods :
* <ul>
*  <li><code> close()</code></li>
* </ul> <p>
* After test object must be recreated.
* @see com.sun.star.sdbc.XCloseable
*/
public class _XCloseable extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XCloseable oObj = null ;

    /**
    * Closes row set. If the component implements the interface
    * <code>com.sun.star.sdbc.XResutlSet</code> then tries to move
    * the cursor to the first row in the result set.
    * Has OK status if no exceptions were thrown during first call and
    * if expected SQL exception was thrown during cursor moving.
    */
    public void _close() throws StatusException {
        boolean res = false;
        try {
            oObj.close();
            res = true;
        } catch (SQLException e) {
            log.println("Unexpected SQL Exception occurred:" + e) ;
            res = false;
        }

        XResultSet resSet = (XResultSet)
            UnoRuntime.queryInterface(XResultSet.class, oObj);

        if (resSet != null) {
            try {
                resSet.first();
                log.println("Expected SQLException not occurred !");
                res = false;
            } catch(SQLException e) {
                log.println("Expected SQLException occurred");
                res = true;
            }
        }

        tRes.tested("close()", res);
    }

    /**
    * Forces environment recreation.
    */
    public void after() {
        disposeEnvironment() ;
    }

}  // finish class _XCloseable

