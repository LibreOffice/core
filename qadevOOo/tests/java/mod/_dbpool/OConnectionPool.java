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

package mod._dbpool;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DBTools;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sdbc.ConnectionPool</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sdbc::XDriverManager</code></li>
* </ul>
* @see com.sun.star.sdbc.XDriverManager
* @see ifc.sdbc.XDriverManager
*/
public class OConnectionPool extends TestCase {
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        XInterface oObj = null;

        try {
            oObj = (XInterface)
                xMSF.createInstance("com.sun.star.sdbc.ConnectionPool");
        } catch(com.sun.star.uno.Exception e) {
            throw new StatusException(
                Status.failed("Couldn't create instance"));
        }

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment( oObj );

        //adding relations for XDriverManager
        String dbaseURL = (String) Param.get("dbase.url");
        if (dbaseURL == null) {
            throw new StatusException(
                Status.failed("Couldn't get parameter 'dbase.url'"));
        }

        tEnv.addObjRelation("SDBC.URL", "sdbc:dbase:" + dbaseURL);

        String jdbcURL = (String) Param.get("jdbc.url");
        if (jdbcURL == null) {
            throw new StatusException(
                Status.failed("Couldn't get parameter 'jdbc.url'"));
        }

        tEnv.addObjRelation("JDBC.URL", "jdbc:" + jdbcURL);

        String jdbcUser = (String) Param.get("jdbc.user");
        if (jdbcUser == null) {
            throw new StatusException(
                Status.failed("Couldn't get parameter 'jdbc.user'"));
        }

        String jdbcPassword = (String) Param.get("jdbc.password");
        if (jdbcPassword == null) {
            throw new StatusException(
                Status.failed("Couldn't get parameter 'jdbc.password'"));
        }

        PropertyValue[] jdbcInfo = new PropertyValue[3];
        jdbcInfo[0] = new PropertyValue();
        jdbcInfo[0].Name = "user";
        jdbcInfo[0].Value = jdbcUser;
        jdbcInfo[1] = new PropertyValue();
        jdbcInfo[1].Name = "password";
        jdbcInfo[1].Value = jdbcPassword;
        jdbcInfo[2] = new PropertyValue();
        jdbcInfo[2].Name = "JavaDriverClass";
        jdbcInfo[2].Value = DBTools.TST_JDBC_DRIVER;

        tEnv.addObjRelation("JDBC.INFO", jdbcInfo);

        return tEnv;
    }
}
