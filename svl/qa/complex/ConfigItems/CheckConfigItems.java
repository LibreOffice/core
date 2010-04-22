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

import com.sun.star.beans.*;
import com.sun.star.lang.*;
import com.sun.star.uno.*;
import com.sun.star.task.*;

import complexlib.*;

import java.lang.*;
import java.util.*;

//-----------------------------------------------
/** @short  todo document me
 */
public class CheckConfigItems extends ComplexTestCase
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
    public String[] getTestMethodNames()
    {
        return new String[]
        {
            "checkPicklist",
            "checkURLHistory",
            "checkHelpBookmarks",
            "checkPrintOptions",
            "checkAccessibilityOptions",
            "checkUserOptions"
        };
    }

    //-------------------------------------------
    /** @short  Create the environment for following tests.

        @descr  Use either a component loader from desktop or
                from frame
     */
    public void before()
        throws java.lang.Exception
    {
        // get uno service manager from global test environment
        m_xSmgr = (XMultiServiceFactory)param.getMSF();

        // TODO register helper service

        // create module manager
        m_xTest = (XJob)UnoRuntime.queryInterface(
                    XJob.class,
                    m_xSmgr.createInstance("com.sun.star.comp.svl.ConfigItemTest"));
    }

    //-------------------------------------------
    /** @short  close the environment.
     */
    public void after()
        throws java.lang.Exception
    {
        // TODO deregister helper service

        m_xTest = null;
        m_xSmgr = null;
    }

    //-------------------------------------------
    /** @todo document me
     */
    public void checkPicklist()
        throws java.lang.Exception
    {
        impl_triggerTest("checkPicklist");
    }

    //-------------------------------------------
    /** @todo document me
     */
    public void checkURLHistory()
        throws java.lang.Exception
    {
        impl_triggerTest("checkURLHistory");
    }

    //-------------------------------------------
    /** @todo document me
     */
    public void checkHelpBookmarks()
        throws java.lang.Exception
    {
        impl_triggerTest("checkHelpBookmarks");
    }

    //-------------------------------------------
    /** @todo document me
     */
    public void checkPrintOptions()
        throws java.lang.Exception
    {
        impl_triggerTest("checkPrintOptions");
    }

    //-------------------------------------------
    /** @todo document me
     */
    public void checkAccessibilityOptions()
        throws java.lang.Exception
    {
        impl_triggerTest("checkAccessibilityOptions");
    }

    //-------------------------------------------
    /** @todo document me
     */
    public void checkUserOptions()
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
}
