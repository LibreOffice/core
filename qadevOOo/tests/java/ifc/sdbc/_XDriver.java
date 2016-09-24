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

import com.sun.star.beans.PropertyValue;
import com.sun.star.sdbc.DriverPropertyInfo;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDriver;

/**
* Testing <code>com.sun.star.sdbc.XDriver</code>
* interface methods :
* <ul>
*  <li><code> connect()</code></li>
*  <li><code> acceptsURL()</code></li>
*  <li><code> getPropertyInfo()</code></li>
*  <li><code> getMajorVersion()</code></li>
*  <li><code> getMinorVersion()</code></li>
* </ul> <p>
* Required object relations :
* <ul>
* <li> <code>'XDriver.URL'</code>:
*      is the URL of the database to which to connect</code></li>
* <li><code>'XDriver.UNSUITABLE_URL'</code>:
*      the wrong kind of URL to connect using given driver</li>
* <li><code>'XDriver.INFO'</code>:
*      a list of arbitrary string tag/value pairs as connection arguments</li>
* </ul> <p>
* @see com.sun.star.sdbc.XDriver
*/
public class _XDriver extends MultiMethodTest {
    // oObj filled by MultiMethodTest
    public XDriver oObj = null;
    String url = null;
    String wrongUrl = null;
    String nbu = null;
    PropertyValue[] info = null;

    /**
    * Retrieves relations.
    * @throw StatusException If any relation not found.
    */
    @Override
    protected void before() {
        nbu = (String) tEnv.getObjRelation("NoBadURL");
        url = (String)tEnv.getObjRelation("XDriver.URL");
        if (url == null) {
            throw new StatusException(Status.failed(
                "Couldn't get relation 'XDriver.URL'"));
        }
        wrongUrl = (String)tEnv.getObjRelation("XDriver.UNSUITABLE_URL");
        if (wrongUrl == null) {
            throw new StatusException(Status.failed(
                "Couldn't get relation 'XDriver.WRONG_URL'"));
        }
        info = (PropertyValue[])tEnv.getObjRelation("XDriver.INFO");
        if (info == null) {
            throw new StatusException(Status.failed(
                "Couldn't get relation 'XDriver.INFO'"));
        }
    }

    /**
    * Connects to <code>'XDriver.URL'</code>,
    * to <code>'XDriver.UNSUITABLE_URL'</code> and to wrong URL using
    * <code>'XDriver.INFO'</code>.
    * Has OK status if the method returns not null for <code>'XDriver.URL'</code>,
    * null for <code>'XDriver.UNSUITABLE_URL'</code> and
    * exception was thrown during the call with a wrong URL.
    */
    public void _connect() {
        boolean res = true;

        try {
            log.println("Trying to connect to " + url);
            XConnection connection = oObj.connect(url, info);
            res = (connection != null);
            log.println("Connected? " + res);
            log.println("Trying to connect to " + wrongUrl);
            connection = oObj.connect(wrongUrl, info);
            res &= (connection == null);
            log.println("Connected? " + !res);
        } catch(SQLException e) {
            log.println("Unexpected exception");
            res &= false;
        }

        if (nbu==null) {
            try {
                String badUrl = url + "bla";
                log.println("Trying to connect to " + badUrl);
                oObj.connect(badUrl, info);
                res &= false;
                log.println("Expected exception isn't thrown");
            } catch(SQLException e) {
                log.println("Expected exception");
                res &= true;
            }
        }

        tRes.tested("connect()", res);
    }

    /**
    * Calls the method for <code>'XDriver.URL'</code> and
    * for <code>'XDriver.UNSUITABLE_URL'</code>.
    * Has OK status if the method returns true for <code>'XDriver.URL'</code>
    * and false for <code>'XDriver.UNSUITABLE_URL'</code>.
    */
    public void _acceptsURL() {
        boolean res = false;

        try {
            res = oObj.acceptsURL(url);
            log.println("Accepts " + url + "? " + res);
            res &= !oObj.acceptsURL(wrongUrl);
            log.println("Accepts " + wrongUrl + "? " + !res);
        } catch(SQLException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("acceptsURL()", res);
    }

    /**
    * Calls the method with passed <code>'XDriver.URL'</code> and
    * <code>'XDriver.INFO'</code>. Prints obtained driver properties info
    * to log.
    * Has OK status if returned value isn't null.
    */
    public void _getPropertyInfo() {
        requiredMethod("acceptsURL()");
        boolean res = false;
        DriverPropertyInfo[] dpi = null;
        try {
            dpi = oObj.getPropertyInfo(url, info);
        } catch(SQLException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res = false;
        }

        if (dpi != null) {
            res = true;
            log.println("Driver properties info:");
            for(int i = 0; i < dpi.length; i++) {
                log.println("Property: " + dpi[i].Name);
                log.println("Description: " + dpi[i].Description);
                log.println("IsRequired? " + dpi[i].IsRequired);
                log.println("Value: " + dpi[i].Value);
                log.println("Choices: ");
                for(int j = 0; j < dpi[i].Choices.length; j++) {
                    log.println("\t" + dpi[i].Choices[j]);
                }
            }
        }

        tRes.tested("getPropertyInfo()", res);
    }

    /**
    * Calls the method.
    * Has OK status if returned value is greater than or is equal to 1.
    */
    public void _getMajorVersion() {
        int majorVer = oObj.getMajorVersion();
        boolean res = majorVer >= 1;
        log.println("Major version " + majorVer);
        tRes.tested("getMajorVersion()", res);
    }

    /**
    * Calls the method.
    * Has OK status if returned value is greater than or is equal to 0.
    */
    public void _getMinorVersion() {
        int minorVer = oObj.getMinorVersion();
        boolean res = minorVer >= 0;
        log.println("Minor version " + minorVer);
        tRes.tested("getMinorVersion()", res);
    }
}