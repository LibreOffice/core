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


package complex.broken_document;

import com.sun.star.beans.PropertyValue;
// import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;


// ---------- junit imports -----------------
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
// ------------------------------------------

/**
 * Check, if message boxes appear when the Office is in "headless" mode. Tests
 * bug i15809. This test uses the broken document dbf.dbf.emf.
 */
public class LoadDocument {

    /** defect file to load **/
    // private final String mFileName = "dbf.dbf.emf";

    /**
     * Get all test methods.
     * @return The test methods.
     */
//    public String[] getTestMethodNames() {
//        return new String[]{"checkHeadlessState"};
//    }

    /**
     * Start Office with "-headless" parameter, then
     * load the broken document "dbf.dbf.emf", that brings a message box up in
     * the ui, see if the headless mode of SOffice changes.
     */
    @Test public void checkHeadlessState()
    {
        XMultiServiceFactory xMSF = getMSF();
        XFrame xDesktop = null;

        try {
            xDesktop = UnoRuntime.queryInterface(XFrame.class, xMSF.createInstance("com.sun.star.frame.Desktop"));
        }
        catch(com.sun.star.uno.Exception e) {
            fail("Could not create a desktop instance.");
        }

        XComponentLoader xDesktopLoader = UnoRuntime.queryInterface(XComponentLoader.class, xDesktop);
        System.out.println("xDektopLoader is null: " + (xDesktopLoader == null));
        PropertyValue[] val = new PropertyValue[0];

        // String workingDir = (String)param.get("WorkingDir") + System.getProperty("file.separator") + mFileName;
        // System.out.println("Working dir: " + workingDir);
        String fileUrl = complex.broken_document.TestDocument.getUrl("dbf.dbf.emf");
        System.out.println("File Url: " + fileUrl);

        try {
            xDesktopLoader.loadComponentFromURL(fileUrl, "_blank", 0, val);
        }
        catch(com.sun.star.io.IOException e) {
            fail("Could not load document");
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            fail("Could not load document");
        }

        // try again: headless mode defect now?
        try {
            xDesktopLoader.loadComponentFromURL(fileUrl, "_blank", 0, val);
        }
        catch(com.sun.star.io.IOException e) {
            fail("Could not load document");
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            fail("Could not load document");
        }

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
