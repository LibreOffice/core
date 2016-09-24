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

import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XIsolatedConnection;
import com.sun.star.task.XInteractionHandler;

/**
 * Testing <code>com.sun.star.sdb.XCompletedConnection</code>
 * interface methods :
 * <ul>
 *  <li><code> getIsolatedConnectionWithCompletion()</code></li>
 *  <li><code> getIsolatedConnection()</code></li>
 * </ul> <p>
*    The following object relations required :
* <ul>
* <li> <code>'XCompletedConnection.Handler'</code> : passed as parameter
* to <code>connectWithCompletion</code> method. </li>
* </ul>
* @see com.sun.star.sdb.XIsolatedConnection
* @see com.sun.star.task.XInteractionHandler
* @see com.sun.star.sdbc.XConnection
*/
public class _XIsolatedConnection extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XIsolatedConnection oObj = null ;

    /**
    * Test call the method with handler passed as object relation.
    * Then value returned is checked.<p>
    * Has OK status if not null value returned. <&nbsp>
    * FAILED if exception occurred, null value returned or object
    * relation was not found.
    */
    public void _getIsolatedConnectionWithCompletion() throws StatusException {
        XInteractionHandler handler = (XInteractionHandler)
            tEnv.getObjRelation("XCompletedConnection.Handler") ;

        if (handler == null) {
            log.println("Required object relation not found !") ;
            tRes.tested("getIsolatedConnectionWithCompletion()", false) ;
            return ;
        }

        XConnection con = null ;
        try {
            con = oObj.getIsolatedConnectionWithCompletion(handler) ;
        } catch (com.sun.star.sdbc.SQLException e) {
            throw new StatusException("Exception while method calling", e) ;
        }

        tRes.tested("getIsolatedConnectionWithCompletion()", con != null) ;
    }

    /**
    * Test call the method with handler passed as object relation.
    * Then value returned is checked.<p>
    * Has OK status if not null value returned. <&nbsp>
    * FAILED if exception occurred, null value returned or object
    * relation was not found.
    */
    public void _getIsolatedConnection() throws StatusException {
        String[] userSettings = (String[])
                        tEnv.getObjRelation("UserAndPassword") ;

        String user = null;
        String pwd = null;

        if (userSettings[0] != null)
            user = userSettings[0].equals("")?"<empty>":userSettings[0];
        else
            user = "<null>";
        if (userSettings[1] != null)
            pwd = userSettings[1].equals("")?"<empty>":userSettings[1];
        else
            pwd = "<null>";

        log.println("Testing \"getIsolatedConnection('user', 'password')\"\n" +
                    "with user = '" + user + "'; password = '" + pwd + "'");
        XConnection con = null ;
        try {
            con = oObj.getIsolatedConnection(user, pwd) ;
        } catch (com.sun.star.sdbc.SQLException e) {
            throw new StatusException("Exception while method calling", e) ;
        }

        tRes.tested("getIsolatedConnection()", con != null) ;
    }
}  // finish class _XIsolatedConnection

