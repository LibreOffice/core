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
package complex.api_internal;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import helper.OfficeProvider;
import helper.ProcessHandler;

import java.io.UnsupportedEncodingException;
import java.util.StringTokenizer;
import lib.TestParameters;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;

import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.task.XJob;
import com.sun.star.uno.UnoRuntime;

/**
 * This test executes the API tests internally in LibreOffice. Prerequisite is
 * that a OOoRunner.jar is registered inside of LibreOffice. Adjust the joblist
 * inside of the CheckAPI.props to determine which tests will be executed.
 */
public class CheckAPI  {

    /**
     * The test parameters
     */
    private TestParameters param = null;

    /**
     *
     */
    @Before public void before()
    {
        param = new TestParameters();
    }
    /**
     * Execute the API tests inside of the Office. If the Office crashes, it
     * will be restarted and the job will continue after the one that caused the crash.
     */
    @Test public void checkAPI() {
        System.out.println("Start with test");
        // if test is idle for 5 minutes, assume that it hangs and kill it.
        XMultiServiceFactory xMSF = getMSF();
        Object oObj = null;
        try {
            oObj = xMSF.createInstance("org.openoffice.RunnerService");
        }
        catch(com.sun.star.uno.Exception e) {
            fail("Could not create Instance of 'org.openoffice.RunnerService'");
        }
        assertNotNull("Cannot create 'org.openoffice.RunnerService'", oObj);

        // get the parameters for the internal test
        final NamedValue[] internalParams = new NamedValue[3];
        internalParams[0] = new NamedValue();
        internalParams[0].Name = "-OutProducer";
        internalParams[0].Value = "stats.SimpleFileOutProducer";
        internalParams[1] = new NamedValue();
        internalParams[1].Name = "-OutputPath";
        internalParams[1].Value = "/dev/null";

        // do we have test jobs?
        final PropertyValue[] props = new PropertyValue[1];
        props[0] = new PropertyValue();
        props[0].Value = "sw.SwXTextTable";

        System.out.println("Props length: "+ props.length);
        for (int i=0; i<props.length; i++) {
            XJob xJob = UnoRuntime.queryInterface(XJob.class, oObj);
            internalParams[2] = new NamedValue();
            internalParams[2].Name = "-o";
            internalParams[2].Value = props[i].Value;
            System.out.println("Executing: " + (String)props[i].Value);

            String erg = null;

            try {
                erg = (String)xJob.execute(internalParams);
            }
            catch(Throwable t) {
                // restart and go on with test!!
                t.printStackTrace();
                fail("Test run '" + (String)props[i].Value +"' could not be executed: Office crashed and is killed!");
                xMSF = null;
                ProcessHandler handler = (ProcessHandler)param.get("AppProvider");
                handler.kill();
                util.utils.pause(10000);
                OfficeProvider op = new OfficeProvider();
                try {
                    xMSF = (XMultiServiceFactory)op.getManager(param);
                    param.put("ServiceFactory",xMSF);

                    oObj = xMSF.createInstance("org.openoffice.RunnerService");
                }
                catch(com.sun.star.uno.Exception e) {
                    fail("Could not create Instance of 'org.openoffice.RunnerService'");
                }
                catch (UnsupportedEncodingException e) {
                    office = null;
                }
            }
            System.out.println(erg);
            String processedErg = parseResult(erg);
            assertTrue("Run '" + (String)props[i].Value + "' has result '" + processedErg + "'", processedErg == null);
        }
    }

    private String parseResult(String erg) {
        String lineFeed = System.getProperty("line.separator");
        String processedErg = null;
        if (erg != null) {
            StringTokenizer token = new StringTokenizer(erg, lineFeed);
            String previousLine = null;
            while ( token.hasMoreTokens() ) {
                String line = token.nextToken();
                // got a failure!
                if ( line.indexOf("FAILED") != -1 ) {
                    processedErg = (processedErg == null)?"":processedErg + ";";
                    processedErg += previousLine + ":" + line;
                }
                if ( line.startsWith("Execute:") ) {
                    previousLine = line;
                }
                else  {
                    previousLine += " " + line;
                }
            }
        }
        return processedErg;
    }


    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}


