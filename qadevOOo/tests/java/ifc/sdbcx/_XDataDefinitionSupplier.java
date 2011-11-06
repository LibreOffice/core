/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package ifc.sdbcx;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.PropertyValue;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDriver;
import com.sun.star.sdbcx.XDataDefinitionSupplier;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sdbcx.XDataDefinitionSupplier</code>
* interface methods :
* <ul>
*  <li><code> getDataDefinitionByConnection()</code></li>
*  <li><code> getDataDefinitionByURL()</code></li>
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
* @see com.sun.star.sdbcx.XDataDefinitionSupplier
*/
public class _XDataDefinitionSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XDataDefinitionSupplier oObj = null ;

    String url = null;
    String wrongUrl = null;
    PropertyValue[] info = null;

    /**
    * Retrieves relations.
    * @throw StatusException If any relation not found.
    */
    protected void before() {
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

    XConnection connection = null;

    /**
     * Obtains the connection to url(relation <code>'XDriver.URL'</code>)
     * with info(relation <code>'XDriver.INFO'</code>).
     * Calls the method with obtained connection and checks that returned value
     * isn't null.
     */
    public void _getDataDefinitionByConnection() {
        boolean bRes = true;
        XDriver xDriver = (XDriver)
            UnoRuntime.queryInterface(XDriver.class, oObj);
        if (xDriver == null) {
            log.println("The XDriver interface isn't supported");
            tRes.tested("getDataDefinitionByConnection()",
                        Status.skipped(false));
            return;
        }
        try {
            connection = xDriver.connect(url, info);
        } catch(com.sun.star.sdbc.SQLException e) {
            e.printStackTrace(log);
            bRes = false;
        }
        if (connection == null) {
            log.println("Couldn't get connection to specified url using " +
                "specified info");
            tRes.tested("getDataDefinitionByConnection()",
                        Status.skipped(false));
            return;
        }
        XTablesSupplier xTS = null;
        try {
            log.println("getDataDefinitionByConnection(connection)");
            xTS = oObj.getDataDefinitionByConnection(connection);
            bRes = xTS != null;
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception: " + e);
            bRes = false;
        }

        try {
            log.println("getDataDefinitionByConnection(null)");
            xTS = oObj.getDataDefinitionByConnection(null);
            bRes = xTS == null;
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Exception: " + e);
            bRes = true;
        }

        tRes.tested("getDataDefinitionByConnection()", bRes);
    }

    /**
     * Calls the method with url and info obtained from the relations
     * <code>XDriver.URL</code> and <code>XDriver.INFO</code>.
     * Checks that retuned value isn't null.
     * Then calls the method with the unsuitable url obtained from the relation
     * <code>XDriver.UNSUITABLE_URL</code> and checks that SQLException
     * exception was thrown.
     */
    public void _getDataDefinitionByURL() {
        boolean bRes = false;
        XTablesSupplier xTS = null;

        try {
            log.println("getDataDefinitionByURL('" + url + "')");
            xTS = oObj.getDataDefinitionByURL(url, info);
            bRes = xTS != null;
        } catch (com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception: " + e);
            bRes = false;
        }

        try {
            log.println("getDataDefinitionByURL('" + wrongUrl + "')");
            xTS = oObj.getDataDefinitionByURL(wrongUrl, info);
            log.println("Exception was expected");
            bRes = false;
        } catch (com.sun.star.sdbc.SQLException e) {
            log.println("Expected exception");
            bRes &= true;
        }

        tRes.tested("getDataDefinitionByURL()", true);

    }
}  // finish class _XDataDefinitionSupplier


