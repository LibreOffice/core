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


package basicrunner;

import lib.TestCase;
import lib.TestParameters;
import lib.TestEnvironment;
import share.DescEntry;
import share.LogWriter;

import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import com.sun.star.beans.PropertyValue;


/**
 * The basic test case.
 */
public class BasicTestCase extends TestCase {

    /**
     * Specifies the PrintWriter to log information.
     */
    public PrintWriter oLog;

    /** The name of the test object **/
    protected String objName;
    /** The implementation name of the test object **/
    protected String implName;
    /** A BasicHandler **/
    static BasicHandler oBasicHandler = null;

    /**
     * Constructor with the entry which is to test.
     * @param entry The description entry.
     */
    public BasicTestCase(DescEntry entry) {
        this.objName = entry.entryName;
        this.implName = entry.longName;
    }


    /**
     * Initialize the test case.
     * The BasicHandler is talken from the test parameters and several
     * parameters are initialized.
     * @param tParam The test parameters.
     * @param pLog A log writer.
     */
    protected void initialize(TestParameters tParam, PrintWriter pLog) {
        // Create Handler ONLY here. If SOffice crashes,
        // no new Handler will be created until new object's initialization.
        this.oLog = pLog;
        LogWriter log = (LogWriter)pLog;
        oBasicHandler = BasicHandlerProvider.getHandler(tParam, log);
        try {
            oBasicHandler.perform("setValue",
                        "cBASPath = \"" + tParam.get("BASICRESPTH") + "/\"");
            oBasicHandler.perform("setValue",
                        "cTestDocsDir = \"" + tParam.get("DOCPTH") + "/\"");
            oBasicHandler.perform("setValue",
                        "CNCSTR = \"" + tParam.get("CNCSTR") + "\"");
            if (tParam.get("soapi.test.hidewindows") != null) {
              oBasicHandler.perform("setValue",
                        "soapi_test_hidewindows = true");
            } else {
              oBasicHandler.perform("setValue",
                        "soapi_test_hidewindows = false");
            }
            //this parameters are used by testcases of db-driver components
            oBasicHandler.perform("setValue", "dbaseUrl = \"sdbc:dbase:" +
                tParam.get("dbase.url") + "\"");
            oBasicHandler.perform("setValue", "flatUrl = \"sdbc:flat:" +
                tParam.get("flat.url") + "\"");
            oBasicHandler.perform("setValue", "calcUrl = \"sdbc:calc:" +
                tParam.get("calc.url") + "\"");
            oBasicHandler.perform("setValue", "odbcUrl = \"sdbc:odbc:" +
                tParam.get("odbc.url") + "\"");
            oBasicHandler.perform("setValue", "jdbcUrl = \"jdbc:" +
                tParam.get("jdbc.url") + "\"");
            oBasicHandler.perform("setValue", "jdbcUser = \"" +
                tParam.get("jdbc.user") + "\"");
            oBasicHandler.perform("setValue", "jdbcPassword = \"" +
                tParam.get("jdbc.password") + "\"");
            oBasicHandler.perform("setValue", "adabasUrl = \"sdbc:adabas:" +
                tParam.get("adabas.url") + "\"");
            oBasicHandler.perform("setValue", "adabasUser = \"" +
                tParam.get("adabas.user") + "\"");
            oBasicHandler.perform("setValue", "adabasPassword = \"" +
                tParam.get("adabas.password") + "\"");
            oBasicHandler.perform("setValue", "adoUrl = \"sdbc:ado:" +
                tParam.get("ado.url") + "\"");
        } catch (BasicException e) {
            log.println(e.info);
            throw new RuntimeException(e.info);
        }
    }

    /**
     * Create the environment for the test. This is done by BASIC.
     * @param tParam The test parameters.
     * @param log A log writer.
     * @return The test environment
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                          PrintWriter log) {

        PropertyValue Res;
        boolean bObjectWasCreated = false;

        try {
            oBasicHandler.perform("setValue",
                        "cObjectImplementationName = \"" + implName + "\"");
            Res = oBasicHandler.perform("createObject", objName);
            bObjectWasCreated = ((Boolean)Res.Value).booleanValue();

            if (!bObjectWasCreated) {
                log.println("Couldn't create object");
                throw new RuntimeException("Couldn't create object");
            }

        } catch (BasicException e) {
            log.println(e.info);
            bObjectWasCreated = false;
            throw new RuntimeException(e.info);
        }

        TestEnvironment tEnv = new TestEnvironment(new XInterface(){});
        tEnv.addObjRelation("objectCreated", new Boolean(bObjectWasCreated));
        tEnv.addObjRelation("BasicHandler", oBasicHandler);
        return tEnv;
    }

    /**
     * BASIC is told to dispose the test object.
     * @param tParam The test parameters.
     */

    public void cleanupTestCase(TestParameters tParam) {
        PropertyValue Res;
        oLog.println("Cleaning up testcase");
        try {
            Res = oBasicHandler.perform("disposeObject", objName);
        } catch (BasicException e) {
            oLog.println(e.info);
            throw new RuntimeException(e.info);
        }
    }

}
