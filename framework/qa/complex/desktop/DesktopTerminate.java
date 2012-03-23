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
package complex.desktop;


import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.frame.XDesktop;
import com.sun.star.uno.UnoRuntime;
import helper.OfficeProvider;

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
 * Parameters:
 * <ul>
 *   <li>NoOffice=yes - StarOffice is not started initially.</li>
 * </ul>
 */
public class DesktopTerminate
{

    private XMultiServiceFactory xMSF;
    private int iOfficeCloseTime = 1000;

    /**
     * A frunction to tell the framework, which test functions are available.
     * Right now, it's only 'checkPersistentWindowState'.
     * @return All test methods.
     */
//    public String[] getTestMethodNames()
//    {
//        return new String[]
//                {
//                    "checkPersistentWindowState"
//                };
//    }

    /**
     * Test if all available document types change the
     * persistent Window Attributes
     *
     * The test follows basically these steps:
     * - Create a configuration reader and a componentloader
     * - Look for all document types in the configuration
     * - Do for every doc type
     *   - start office
     *   - read configuration attibute settings
     *   - create a new document
     *   - resize the document and close it
     *   - close office
     *   - start office
     *   - read configuration attribute settings
     *   - create another new document
     *   - compare old settings with new ones: should be different
     *   - compare the document size with the resized document: should be equal
     *   - close office
     * - Test finished
     */
    @Test public void checkPersistentWindowState()
    {
        try
        {

            System.out.println("Connect the first time.");
//            System.out.println("AppExecCommand: " + (String) param.get("AppExecutionCommand"));
//            System.out.println("ConnString: " + (String) param.get("ConnectionString"));
//            oProvider = new OfficeProvider();
//            iOfficeCloseTime = param.getInt("OfficeCloseTime");
//            if (iOfficeCloseTime == 0)
//            {
//                iOfficeCloseTime = 1000;
//            }

            if (!connect())
            {
                return;
            }

            if (!disconnect())
            {
                return;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    private boolean connect()
    {
        try
        {
            xMSF = getMSF();
            try
            {
                Thread.sleep(10000);
            }
            catch (java.lang.InterruptedException e)
            {
            }
        }
        catch (java.lang.Exception e)
        {
            System.out.println(e.getClass().getName());
            System.out.println("Message: " + e.getMessage());
            fail("Cannot connect the Office.");
            return false;
        }
        return true;
    }

    private boolean disconnect()
    {
        try
        {
            XDesktop desk = null;
            desk = UnoRuntime.queryInterface(XDesktop.class, xMSF.createInstance("com.sun.star.frame.Desktop"));
            desk.terminate();
            System.out.println("Waiting " + iOfficeCloseTime + " milliseconds for the Office to close down");
            try
            {
                Thread.sleep(iOfficeCloseTime);
            }
            catch (java.lang.InterruptedException e)
            {
            }
            xMSF = null;
        }
        catch (java.lang.Exception e)
        {
            e.printStackTrace();
            fail("Cannot dispose the Office.");
            return false;
        }
        return true;
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
        // don't do a tearDown here, desktop is already terminated.
        // connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
