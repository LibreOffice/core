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
package complex.disposing;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XDesktop;

import org.junit.AfterClass;
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
