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
// package name: as default, start with complex
package complex.api_internal;

// imports
import helper.OfficeProvider;
import helper.ProcessHandler;
import com.sun.star.task.XJob;
import com.sun.star.beans.XPropertyAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.NamedValue;

import java.util.Vector;
import java.util.StringTokenizer;


// ---------- junit imports -----------------
import lib.TestParameters;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
// ------------------------------------------

/**
 * This test executes the API tests internally in StarOffice. Prerequiste is
 * that a OOoRunner.jar is registered inseide of StarOffice. Adjust the joblist
 * inside of the ChekAPI.props to determine which tetss will be executed.
 */
public class CheckAPI  {

    // The name of the tested service
    private final String testName = "StarOfficeAPI";

    /**
     * Return all test methods.
     * @return The test methods.
     */
//    public String[] getTestMethodNames() {
//        return new String[]{"checkAPI"};
//    }

    /**
     * The test parameters
     */
    private static TestParameters param = null;

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
        // param.put("TimeOut", new Integer("300000"));
/*        AppProvider office = (AppProvider)dcl.getInstance("helper.OfficeProvider");
        Object msf = office.getManager(param);
        if (msf == null) {
            failed("Could not connect an Office.");
        }
        param.put("ServiceFactory",msf); */
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
        String paramList = (String)param.get("ParamList");
        Vector p = new Vector();
        StringTokenizer paramTokens = new StringTokenizer(paramList, " ");
        while(paramTokens.hasMoreTokens())
        {
            p.add(paramTokens.nextToken());
        }
        int length = p.size()/2+1;
        NamedValue[] internalParams = new NamedValue[length];
        for (int i=0; i<length-1; i++) {
            internalParams[i] = new NamedValue();
            internalParams[i].Name = (String)p.get(i*2);
            internalParams[i].Value = p.get(i*2+1);
            System.out.println("Name: "+internalParams[i].Name);
            System.out.println("Value: "+(String)internalParams[i].Value);
        }

        // do we have test jobs?
        String testJob = (String)param.get("job");
        PropertyValue[]props;
        if (testJob==null)
        {
            if ( param.get("job1")==null )
            {
                // get all test jobs from runner service
                XPropertyAccess xPropAcc = (XPropertyAccess)UnoRuntime.queryInterface(XPropertyAccess.class, oObj);
                props = xPropAcc.getPropertyValues();
            }
            else  {
                int index=1;
                p = new Vector();
                while ( param.get("job"+index) != null ) {
                    p.add(param.get("job"+index));
                    index++;
                }
                props = new PropertyValue[p.size()];
                for ( int i=0; i<props.length; i++ ) {
                    props[i] = new PropertyValue();
                    props[i].Value = p.get(i);
                }
            }
        }
        else  {
            props = new PropertyValue[1];
            props[0] = new PropertyValue();
            props[0].Value = testJob;
        }

        System.out.println("Props length: "+ props.length);
        for (int i=0; i<props.length; i++) {
            XJob xJob = UnoRuntime.queryInterface(XJob.class, oObj);
            internalParams[length-1] = new NamedValue();
            internalParams[length-1].Name = "-o";
            internalParams[length-1].Value = props[i].Value;
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
                try {
                    Thread.sleep(10000);
                }
                catch(java.lang.InterruptedException e) {}
                OfficeProvider op = new OfficeProvider();
                //                op.closeExistingOffice(param, true);
                xMSF = (XMultiServiceFactory)op.getManager(param);
                param.put("ServiceFactory",xMSF);
                try {
                    oObj = xMSF.createInstance("org.openoffice.RunnerService");
                }
                catch(com.sun.star.uno.Exception e) {
                    fail("Could not create Instance of 'org.openoffice.RunnerService'");
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
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
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


