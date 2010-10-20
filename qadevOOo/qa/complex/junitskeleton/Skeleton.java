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
package complex.junitskeleton;

import com.sun.star.io.IOException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import java.io.File;
import java.io.RandomAccessFile;

import lib.TestParameters;

import util.SOfficeFactory;

// ---------- junit imports -----------------
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
// ------------------------------------------

public class Skeleton
{
    /**
     * The test parameters
     */
    private static TestParameters param = null;

    @Test public void check() {
        assertTrue("Couldn't open document", open());
        System.out.println("check");
        assertTrue("Couldn't close document", close());
        String tempDirURL = util.utils.getOfficeTemp/*Dir*/(getMSF());
        System.out.println("temp dir URL is: " + tempDirURL);
        String tempDir = graphical.FileHelper.getSystemPathFromFileURL(tempDirURL);
        assertTrue("Temp directory doesn't exist.", new File(tempDir).exists());
    }

    private boolean open()
        {
            System.out.println("open()");
            // get multiservicefactory -----------------------------------------
            final XMultiServiceFactory xMsf = getMSF();

            SOfficeFactory SOF = SOfficeFactory.getFactory(xMsf);

            // some Tests need the qadevOOo TestParameters, it is like a Hashmap for Properties.
            param = new TestParameters();
            param.put("ServiceFactory", xMsf); // some qadevOOo functions need the ServiceFactory

            return true;
        }

    private boolean close()
        {
            System.out.println("close()");
            return true;
        }

    // marked as test
    @Test public void checkDocument()
        {
            System.out.println("checkDocument()");
            final String sREADME = TestDocument.getUrl("README.txt");
            System.out.println("README is in:" + sREADME);
            File aFile = new File(sREADME);
            if (! aFile.exists())
            {
                // It is a little bit stupid that office urls not compatible to java file urls
                System.out.println("java.io.File can't access Office file urls.");
                String sREADMESystemPath = graphical.FileHelper.getSystemPathFromFileURL(sREADME);
                aFile = new File(sREADMESystemPath);
                assertTrue("File '" + sREADMESystemPath + "' doesn't exists.", aFile.exists());
            }

            try
            {
                RandomAccessFile aAccess = new RandomAccessFile(aFile, "r");
                long nLength = aAccess.length();
                System.out.println("File length: " + nLength);
                assertTrue("File length wrong", nLength > 0);
                String sLine = aAccess.readLine();
                assertTrue("Line must not be empty", sLine.length() > 0);
                System.out.println("       Line: '" + sLine + "'");
                System.out.println("     length: " + sLine.length());
                assertTrue("File length not near equal to string length", sLine.length() + 2 >= nLength);
                aAccess.close();
            }
            catch (java.io.FileNotFoundException e)
            {
                fail("Can't find file: " + sREADME + " - " + e.getMessage());
            }
            catch (java.io.IOException e)
            {
                fail("IO Exception: " + e.getMessage());
            }

        }

    @Test public void checkOpenDocumentWithOffice()
    {
        // SOfficeFactory aFactory = new SOfficeFactory(getMSF());
        SOfficeFactory SOF = SOfficeFactory.getFactory(getMSF());
        final String sREADME = TestDocument.getUrl("README.txt");
        try
        {
            XComponent aDocument = SOF.loadDocument(sREADME);
            complex.junitskeleton.justatest.shortWait();
            XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, aDocument);
            xClose.close(true);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex)
        {
            fail("Illegal argument exception caught: " + ex.getMessage());
        }
        catch (com.sun.star.io.IOException ex)
        {
            fail("IOException caught: " + ex.getMessage());
        }
        catch (com.sun.star.uno.Exception ex)
        {
            fail("Exception caught: " + ex.getMessage());
        }
    }

    // marked as prepare for test, will call before every test
    @Before public void before()
        {
            System.out.println("before()");
            System.setProperty("THIS IS A TEST", "Hallo");
        }


    // marked as post for test, will call after every test
    @After public void after()
        {
            System.out.println("after()");
            String sValue = System.getProperty("THIS IS A TEST");
            assertEquals(sValue, "Hallo");
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
