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
    * <code>com.sun.star.sdbc.XResultSet</code> then tries to move
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

        XResultSet resSet = UnoRuntime.queryInterface(XResultSet.class, oObj);

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
    @Override
    public void after() {
        disposeEnvironment() ;
    }

}  // finish class _XCloseable

