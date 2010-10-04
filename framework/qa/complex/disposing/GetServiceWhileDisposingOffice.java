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
package complex.disposing;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XDesktop;

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
 * This test is for bug110698. The Office is closed and is continually connected
 * while it closes. This did let the Office freeze. Now when the Office is
 * closed, the connection is refused.
 */
public class GetServiceWhileDisposingOffice
{

//    public String[] getTestMethodNames()
//    {
//        return new String[]
//                {
//                    "checkServiceWhileDisposing"
//                };
//    }

    @Test public void checkServiceWhileDisposing()
    {
        XMultiServiceFactory xMSF = getMSF();
        XDesktop xDesktop = null;

        try
        {
            xDesktop = UnoRuntime.queryInterface(XDesktop.class, xMSF.createInstance("com.sun.star.frame.Desktop"));
        }
        catch (com.sun.star.uno.Exception e)
        {
            fail("Could not create a desktop instance.");
        }
        int step = 0;
        try
        {
            System.out.println("Start the termination of the Office.");
            xDesktop.terminate();
            for (; step < 10000; step++)
            {
                Object o = xMSF.createInstance("com.sun.star.frame.Desktop");
            }
        }
        catch (com.sun.star.lang.DisposedException e)
        {
            System.out.println("DisposedException in step: " + step);
        }
        catch (Exception e)
        {
            fail(e.getMessage());
        }

    }


       private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        // Office is already terminated.
        // connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();

}
