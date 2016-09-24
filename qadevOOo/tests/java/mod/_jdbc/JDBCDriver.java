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

package mod._jdbc;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DBTools;

import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.XInterface;


/**
* Here <code>com.sun.star.sdbc.Driver</code> service is tested.<p>
* Test allows to run object tests in several threads concurrently.
* @see com.sun.star.sdbc.Driver
* @see com.sun.star.sdbc.XDriver
* @see ifc.sdbc._XDriver
*/
public class JDBCDriver extends TestCase {
    /**
     * Creates an instance of the service
     * <code>com.sun.star.sdbc.Driver</code>. <p>
     * Object relations created :
     * <ul>
     *  <li> <code>'XDriver.URL'</code> for {@link ifc.sdbc._XDriver}:
     *      is the URL of the database to which to connect.
     *      The URL is obtained from the parameter <code>jdbc.url</code></li>
     *  <li> <code>'XDriver.UNSUITABLE_URL'</code> for {@link ifc.sdbc._XDriver}:
     *      the wrong kind of URL to connect using given driver.
     *      The URL is obtained from the parameter <code>flat.url</code></li>
     *  <li> <code>'XDriver.INFO'</code> for {@link ifc.sdbc._XDriver}:
     *      a list of arbitrary string tag/value pairs as connection arguments.
     *      The values for list are obtained from the parameter
     *      <code>jdbc.user</code> and <code>jdbc.password</code>.</li>
     * </ul>
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = (XInterface)Param.getMSF().createInstance(
            "com.sun.star.comp.sdbc.JDBCDriver");

        log.println("creating a new environment for JDBCDriver object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        //adding relation for sdbc.XDriver
        String jdbcURL = (String) Param.get("jdbc.url");
        if (jdbcURL == null) {
            throw new StatusException(Status.failed(
                "Couldn't get 'jdbc.url' from ini-file"));
        }
        tEnv.addObjRelation("XDriver.URL", "jdbc:" + jdbcURL);

        String user = (String) Param.get("jdbc.user");
        String password = (String) Param.get("jdbc.password");
        if (user == null || password == null) {
            throw new StatusException(Status.failed(
                "Couldn't get 'jdbc.user' or 'jdbc.password' from ini-file"));
        }
        PropertyValue[] info = new PropertyValue[4];
        info[0] = new PropertyValue();
        info[0].Name = "JavaDriverClass";
        info[0].Value = DBTools.TST_JDBC_DRIVER;
        info[1] = new PropertyValue();
        info[1].Name = "user";
        info[1].Value = user;
        info[2] = new PropertyValue();
        info[2].Name = "password";
        info[2].Value = password;
        info[3] = new PropertyValue();
        info[3].Name = "isPasswordRequired";
        info[3].Value = Boolean.TRUE;

        tEnv.addObjRelation("XDriver.INFO", info);

        String flatUrl = (String) Param.get("flat.url");
        if (flatUrl == null) {
            throw new StatusException(Status.failed(
                "Couldn't get 'flat.url' from ini-file"));
        }
        tEnv.addObjRelation("XDriver.UNSUITABLE_URL", "sdbc:flat:" + flatUrl);

        return tEnv;
    }
}
