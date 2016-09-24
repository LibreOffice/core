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

import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDataSource;

/**
* Testing <code>com.sun.star.sdbc.XDataSource</code>
* interface methods :
* <ul>
*  <li><code>getConnection()</code></li>
*  <li><code>setLoginTimeout()</code></li>
*  <li><code>getLoginTimeout()</code></li>
* </ul> <p>
* @see com.sun.star.sdbc.XDataSource
*/
public class _XDataSource extends MultiMethodTest {
    // oObj filled by MultiMethodTest
    public XDataSource oObj = null;

    /**
     * Calls the method and checks returned value.
     * Has OK status if exception wasn't thrown and
     * if returned value isn't null.
     */
    public void _getConnection() {
        boolean res = true;

        try {
            XConnection connection = oObj.getConnection("", "");
            res = connection != null;
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception:");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("getConnection()", res);
    }

    /**
     * Sets new timeout, compares with timeout returned by the method
     * <code>getLoginTimeout()</code>.
     * Has OK status if exception wasn't thrown and if timeout values are equal.
     */
    public void _setLoginTimeout() {
        requiredMethod("getLoginTimeout()");
        boolean res = true;

        try {
            final int TO = 111;
            log.println("setLoginTimeout(" + TO +  ")");
            oObj.setLoginTimeout(TO);
            int timeout = oObj.getLoginTimeout();
            res = timeout == TO;
            log.println("getLoginTimeout(): " + timeout);
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception:");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("setLoginTimeout()", res);
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if exception wasn't thrown and
     * if returned value is equal to zero.
     */
    public void _getLoginTimeout() {
        boolean res = true;

        try {
            int timeout = oObj.getLoginTimeout();
            log.println("getLoginTimeout(): " + timeout);
            res = timeout == 0;
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception:");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("getLoginTimeout()", res);
    }
}