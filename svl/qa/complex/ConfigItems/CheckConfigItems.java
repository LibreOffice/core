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

package complex.ConfigItems;

import com.sun.star.beans.NamedValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.task.XJob;
import com.sun.star.uno.UnoRuntime;


import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
// import static org.junit.Assert.*;

//-----------------------------------------------
/** @short  todo document me
 * @deprecated this tests seems no longer work as expected.
 */
public class CheckConfigItems
{
    //-------------------------------------------
    // some const

    //-------------------------------------------
    // member

    /** points to the global uno service manager. */
    private XMultiServiceFactory m_xSmgr = null;

    /** implements real config item tests in C++. */
    private XJob m_xTest = null;

    //-------------------------------------------
    // test environment

    //-------------------------------------------
    /** @short  A function to tell the framework,
                which test functions are available.

        @return All test methods.
        @todo   Think about selection of tests from outside ...
     */
//    public String[] getTestMethodNames()
//    {
//        return new String[]
//        {
//            "checkPicklist",
//            "checkURLHistory",
//            "checkHelpBookmarks",
//            "checkPrintOptions",
//            "checkAccessibilityOptions",
//          "checkUserOptions"
//        };
//    }

    //-------------------------------------------
    /** @short  Create the environment for following tests.

     * @throws java.lang.Exception
     * @descr  Use either a component loader from desktop or
                from frame
     */
    @Before public void before()
        throws java.lang.Exception
    {
        // get uno service manager from global test environment
        m_xSmgr = getMSF();

        // TODO register helper service

        // create module manager
        m_xTest = UnoRuntime.queryInterface(XJob.class, m_xSmgr.createInstance("com.sun.star.comp.svl.ConfigItemTest"));
    }

    //-------------------------------------------
    /**
     * @throws java.lang.Exception
     * @short  close the environment.
     */
    @After public void after()
        throws java.lang.Exception
    {
        // TODO deregister helper service

        m_xTest = null;
        m_xSmgr = null;
    }

    //-------------------------------------------
    /**
     * @throws java.lang.Exception
     * @todo document me
     */
    @Test public void checkPicklist()
        throws java.lang.Exception
    {
        impl_triggerTest("checkPicklist");
    }

    //-------------------------------------------
    /**
     * @throws java.lang.Exception
     * @todo document me
     */
    @Test public void checkURLHistory()
        throws java.lang.Exception
    {
        impl_triggerTest("checkURLHistory");
    }

    //-------------------------------------------
    /**
     * @throws java.lang.Exception
     * @todo document me
     */
    @Test public void checkHelpBookmarks()
        throws java.lang.Exception
    {
        impl_triggerTest("checkHelpBookmarks");
    }

    //-------------------------------------------
    /**
     * @throws java.lang.Exception
     * @todo document me
     */
//     @Test public void checkPrintOptions()
//         throws java.lang.Exception
//     {
//         impl_triggerTest("checkPrintOptions");
//     }

    //-------------------------------------------
    /**
     * @throws java.lang.Exception
     * @todo document me
     */
    @Test public void checkAccessibilityOptions()
        throws java.lang.Exception
    {
        impl_triggerTest("checkAccessibilityOptions");
    }

    //-------------------------------------------
    /**
     * @throws java.lang.Exception
     * @todo document me
     */
    @Test public void checkUserOptions()
        throws java.lang.Exception
    {
        impl_triggerTest("checkUserOptions");
    }

    //-------------------------------------------
    /** @todo document me
     */
    private void impl_triggerTest(String sTest)
        throws java.lang.Exception
    {
        NamedValue[] lArgs          = new NamedValue[1];
                     lArgs[0]       = new NamedValue();
                     lArgs[0].Name  = "Test";
                     lArgs[0].Value = sTest;
        m_xTest.execute(lArgs);
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
