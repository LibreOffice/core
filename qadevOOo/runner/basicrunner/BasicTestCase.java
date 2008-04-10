/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BasicTestCase.java,v $
 * $Revision: 1.8 $
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
package basicrunner;

import lib.TestCase;
import lib.TestParameters;
import lib.TestEnvironment;
import share.DescEntry;
import util.SOfficeFactory;
import share.LogWriter;
import stats.SimpleLogWriter;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.document.XDocumentInfo;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.uno.Exception;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.File;
import java.io.FileWriter;
import java.io.FileReader;
import java.io.PrintWriter;

import java.util.Vector;

import com.sun.star.uno.XInterface;
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
            oBasicHandler.perform("setValue", "mozabUrl = \"sdbc:address:" +
                tParam.get("mozab.url") + "\"");
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


    protected void cleanupTestEnvironment(TestParameters tParam,
            TestEnvironment tEnv, LogWriter log) {
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
