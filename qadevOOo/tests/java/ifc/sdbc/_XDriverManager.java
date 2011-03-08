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
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.PropertyValue;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDriverManager;

/**
* Testing <code>com.sun.star.sdbc.XDriverManager</code>
* interface methods :
* <ul>
*  <li><code> getConnection()</code></li>
*  <li><code> getConnectionWithInfo()</code></li>
*  <li><code> setLoginTimeout()</code></li>
*  <li><code> getLoginTimeout()</code></li>
* </ul> <p>
* Required object relations :
* <ul>
* <li> <code>'SDBC.URL'</code>:
*      is the URL of the database to which to connect using sdbc-driver
* </code></li>
* <li> <code>'JDBC.URL'</code>:
*      is the URL of the database to which to connect using jdbc-driver
* </code></li>
* <li> <code>'JDBC.INFO'</code> of type <code>PropertyValue[]</code>:
*      a list of arbitrary string tag/value pairs as connection arguments;
*      normally at least a "user" and "password" property should be included
* </code></li>
* </ul> <p>
* @see com.sun.star.sdbc.XDriverManager
*/
public class _XDriverManager extends MultiMethodTest {
    // oObj filled by MultiMethodTest
    public XDriverManager oObj = null;
    String sdbcURL = null;
    String jdbcURL = null;
    PropertyValue[] jdbcINFO = null;

    /**
     * Retrieves the required object relations.
     */
    protected void before() {
        sdbcURL = (String)tEnv.getObjRelation("SDBC.URL");
        if (sdbcURL == null) {
            throw new StatusException(
                Status.failed("Couldn't get relation 'SDBC.URL'"));
        }
        jdbcURL = (String)tEnv.getObjRelation("JDBC.URL");
        if (jdbcURL == null) {
            throw new StatusException(
                Status.failed("Couldn't get relation 'JDBC.URL'"));
        }
        jdbcINFO = (PropertyValue[])tEnv.getObjRelation("JDBC.INFO");
        if (jdbcINFO == null) {
            throw new StatusException(
                Status.failed("Couldn't get relation 'JDBC.INFO'"));
        }
    }

    /**
     * Calls the method with the url received from the relation
     * <code>SDBC.URL</code>.
     * Has OK status if exception wasn't thrown and
     * if returned value isn't null.
     */
    public void _getConnection() {
        boolean res = true;

        try {
            log.println("getConnection(" + sdbcURL + ")");
            XConnection connection = oObj.getConnection(sdbcURL);
            res = connection != null;
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("getConnection()", res);
    }

    /**
     * Calls the method with the url received from the relation
     * <code>JDBC.URL</code> and with info received from the relation
     * <code>JDBC.INFO</code>.
     * Has OK status if exception wasn't thrown and
     * if returned value isn't null.
     */
    public void _getConnectionWithInfo() {
        boolean res = true;

        try {
            log.println("getConnectionWithInfo(" + jdbcURL + ")");
            XConnection connection =
                oObj.getConnectionWithInfo(jdbcURL, jdbcINFO);
            res = connection != null;
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("getConnectionWithInfo()", res);
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if timeout that was set and timeout that was returned by
     * the method <code>getLoginTimeout()</code> are equal.
     */
    public void _setLoginTimeout() {
        requiredMethod("getLoginTimeout()");
        final int TO = 111;
        log.println("setLoginTimeout(" + TO + ")");
        oObj.setLoginTimeout(TO);
        int timeout = oObj.getLoginTimeout();
        log.println("getLoginTimeout(): " + timeout);
        tRes.tested("setLoginTimeout()", timeout == TO);
    }

    /**
     * Calls the method.
     */
    public void _getLoginTimeout() {
        int timeout = oObj.getLoginTimeout();
        log.println("getLoginTimeout(): " + timeout);

        tRes.tested("getLoginTimeout()", true);
    }
}