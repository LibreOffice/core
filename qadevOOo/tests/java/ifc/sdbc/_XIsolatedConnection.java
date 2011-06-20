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
        if (userSettings == null) {
            log.println("Required object relation not found !") ;
        }

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

