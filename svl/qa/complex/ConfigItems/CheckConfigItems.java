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


/** @short  todo document me
 * @deprecated this tests seems no longer work as expected.
 */
public class CheckConfigItems
{

    // some const


    // member

    /** points to the global uno service manager. */
    private XMultiServiceFactory m_xSmgr = null;

    /** implements real config item tests in C++. */
    private XJob m_xTest = null;


    // test environment


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


    /**
     * @throws java.lang.Exception
     * @todo document me
     */
    @Test public void checkPicklist()
        throws java.lang.Exception
    {
        impl_triggerTest("checkPicklist");
    }


    /**
     * @throws java.lang.Exception
     * @todo document me
     */
    @Test public void checkURLHistory()
        throws java.lang.Exception
    {
        impl_triggerTest("checkURLHistory");
    }


    /**
     * @throws java.lang.Exception
     * @todo document me
     */
    @Test public void checkHelpBookmarks()
        throws java.lang.Exception
    {
        impl_triggerTest("checkHelpBookmarks");
    }


    /**
     * @throws java.lang.Exception
     * @todo document me
     */
//     @Test public void checkPrintOptions()
//         throws java.lang.Exception
//     {
//         impl_triggerTest("checkPrintOptions");
//     }


    /**
     * @throws java.lang.Exception
     * @todo document me
     */
    @Test public void checkAccessibilityOptions()
        throws java.lang.Exception
    {
        impl_triggerTest("checkAccessibilityOptions");
    }


    /**
     * @throws java.lang.Exception
     * @todo document me
     */
    @Test public void checkUserOptions()
        throws java.lang.Exception
    {
        impl_triggerTest("checkUserOptions");
    }


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
