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

package mod._file.dbase;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
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
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.sdbc.Driver</code>. <p>
     * Object relations created :
     * <ul>
     *  <li> <code>'XDriver.URL'</code> for {@link ifc.sdbc._XDriver}:
     *      is the URL of the database to which to connect.
     *      The URL is obtained from the parameter <code>dbase.url</code></li>
     *  <li> <code>'XDriver.UNSUITABLE_URL'</code> for
     *  {@link ifc.sdbc._XDriver}:
     *      the wrong kind of URL to connect using given driver.
     *      The URL is obtained from the parameter <code>jdbc.url</code></li>
     *  <li> <code>'XDriver.INFO'</code> for {@link ifc.sdbc._XDriver}:
     *      a list of arbitrary string tag/value pairs as connection arguments
     *      </li>
     * </ul>
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        try {
            oObj = (XInterface)(
                (XMultiServiceFactory)Param.getMSF()).createInstance(
                "com.sun.star.comp.sdbc.dbase.ODriver");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException(Status.failed("Couldn't create object"));
        }

        log.println("creating a new environment for dbase.ODriver object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        //adding relation for sdbc.XDriver
        String dBaseURL = (String) Param.get("dbase.url");
        if (dBaseURL == null) {
            throw new StatusException(Status.failed(
                "Couldn't get 'dbase.url' from ini-file"));
        }
        tEnv.addObjRelation("XDriver.URL", "sdbc:dbase:" + dBaseURL);

        PropertyValue[] info = new PropertyValue[0];
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
