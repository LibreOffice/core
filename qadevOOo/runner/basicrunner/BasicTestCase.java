/*************************************************************************
 *
 *  $RCSfile: BasicTestCase.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-11-18 16:13:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            helper.ProcessHandler ph =
                            (helper.ProcessHandler) tParam.get("AppProvider");

            if (ph != null) ph.kill();
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
            }

        } catch (BasicException e) {
            log.println(e.info);
            bObjectWasCreated = false;
        }

        TestEnvironment tEnv = new TestEnvironment(new XInterface(){});
        tEnv.addObjRelation("objectCreated", Boolean.valueOf(bObjectWasCreated));
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
        }
    }

}
