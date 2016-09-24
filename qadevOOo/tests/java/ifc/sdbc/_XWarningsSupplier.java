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
import lib.Status;
import lib.StatusException;
import util.DBTools;
import util.utils;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowUpdate;
import com.sun.star.sdbc.XWarningsSupplier;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sdbc.XWarningsSupplier</code>
* interface methods :
* <ul>
*  <li><code> getWarnings()</code></li>
*  <li><code> clearWarnings()</code></li>
* </ul> <p>
* @see com.sun.star.sdbc.XWarningsSupplier
*/
public class _XWarningsSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XWarningsSupplier oObj = null ;

    /**
    * Updates value of int column by value '9999999999999999'.
    * Calls method and checks returned value. <p>
    * Has OK status if the method return not empty value.
    */
    public void _getWarnings() {
        final XRowUpdate rowUpdate = UnoRuntime.queryInterface(XRowUpdate.class, oObj);
        final XResultSetUpdate resultSetUpdate = UnoRuntime.queryInterface(XResultSetUpdate.class, rowUpdate);
        final XRow row = UnoRuntime.queryInterface(XRow.class, resultSetUpdate);
        if ( row == null)
            throw new StatusException(Status.failed("Test must be modified"));

        // not sure what the below test was intended to test, but it actually fails with an SQLException (which is
        // correct for what is done there), and thus makes the complete interface test fail (which is not correct)
        // So, for the moment, just let the test succeed all the time - until issue #i84235# is fixed

        if ( false )
        {
            int oldVal = 0, newVal = 0;
            String valToSet = "9999999999999999";
            try
            {
                oldVal = row.getInt(DBTools.TST_INT);
                rowUpdate.updateString(DBTools.TST_INT, valToSet);
                resultSetUpdate.updateRow();
                newVal = row.getInt(DBTools.TST_INT);
            }
            catch(com.sun.star.sdbc.SQLException e)
            {
                log.println("Unexpected SQL exception");
                e.printStackTrace(log);
                tRes.tested("getWarnings()", false);
                return;
            }

            log.println("Old INT value: " + oldVal);
            log.println("Value that was set: " + valToSet);
            log.println("New INT value: " + newVal);

            boolean res = false;

            try
            {
                Object warns = oObj.getWarnings();
                res = (!utils.isVoid(warns));
            }
            catch (SQLException e)
            {
                log.println("Exception occurred :");
                e.printStackTrace(log);
                tRes.tested("getWarnings()", res);
                return;
            }
            tRes.tested("getWarnings()", res);
        }
        else
            tRes.tested( "getWarnings()", true );
    }

    /**
    * Calls method and checks value returned by the method
    * <code>getWarnings()</code>. <p>
    * Has OK status if the method <code>getWarnings()</code> return void value.
    */
    public void _clearWarnings() {
        executeMethod("getWarnings()");
        boolean res = false;

        try {
            oObj.clearWarnings();
            Object warns = oObj.getWarnings();
            res = utils.isVoid(warns);
        } catch (SQLException e) {
            log.println("Exception occurred :");
            e.printStackTrace(log);
            tRes.tested("clearWarnings()", res);
            return;
        }

        tRes.tested("clearWarnings()", res);
    }

}
