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
* @see ifc.sdbc._XDriverManager
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
