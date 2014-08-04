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

package mod._adabas;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.XInterface;

/**
* Here <code>com.sun.star.sdbc.Driver</code> service is tested.<p>
* Test allows to run object tests in several threads concurently.
* @see com.sun.star.sdbc.Driver
* @see com.sun.star.sdbc.XDriver
* @see com.sun.star.sdbcx.XCreateCatalog
* @see com.sun.star.sdbcx.XDropCatalog
* @see ifc.sdbc._XDriver
* @see ifc.sdbcx._XCreateCatalog
* @see ifc.sdbcx._XDropCatalog
*/
public class ODriver extends TestCase {
    /**
     * Creates an instance of the service
     * <code>com.sun.star.sdbc.Driver</code>. <p>
     * Object relations created :
     * <ul>
     *  <li> <code>'XDriver.URL'</code> for {@link ifc.sdbc._XDriver}:
     *      is the URL of the database to which to connect.
     *      The URL is obtained from the parameter <code>adabas.url</code></li>
     *  <li> <code>'XDriver.UNSUITABLE_URL'</code> for {@link ifc.sdbc._XDriver}:
     *      the wrong kind of URL to connect using given driver.
     *      The URL is obtained from the parameter <code>jdbc.url</code></li>
     *  <li> <code>'XDriver.INFO'</code> for {@link ifc.sdbc._XDriver}:
     *      a list of arbitrary string tag/value pairs as connection arguments.
     *      The values for list are obtained from the parameter
     *      <code>adabas.user</code> and <code>adabas.password</code>.</li>
     * </ul>
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        try {
            oObj = (XInterface)Param.getMSF().
                createInstance("com.sun.star.comp.sdbcx.adabas.ODriver");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed("Couldn't create object"));
        }

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        //adding relation for sdbc.XDriver
        String adabasURL = (String) Param.get("adabas.url");
        if (adabasURL == null) {
            throw new StatusException(Status.failed(
                "Couldn't get 'adabas.url' from ini-file"));
        }
        tEnv.addObjRelation("XDriver.URL", "sdbc:adabas:" + adabasURL);


        String user = (String) Param.get("adabas.user");
        if (user == null) {
            throw new StatusException(Status.failed(
                "Couldn't get 'adabas.user' from ini-file"));
        }
        String password = (String) Param.get("adabas.password");
        if (password == null) {
            throw new StatusException(Status.failed(
                "Couldn't get 'adabas.password' from ini-file"));
        }
        PropertyValue[] info = new PropertyValue[2];
        info[0] = new PropertyValue();
        info[0].Name = "user"; info[0].Value = user;
        info[1] = new PropertyValue();
        info[1].Name = "password"; info[1].Value = password;
        tEnv.addObjRelation("XDriver.INFO", info);

        String jdbcUrl = (String) Param.get("jdbc.url");
        if (jdbcUrl == null) {
            throw new StatusException(Status.failed(
                "Couldn't get 'jdbc.url' from ini-file"));
        }
        tEnv.addObjRelation("XDriver.UNSUITABLE_URL", "jdbc:" + jdbcUrl);

        return tEnv;
    }
}
