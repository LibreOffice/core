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
package complex.dispatches;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.DispatchInformation;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDispatchInformationProvider;
import com.sun.star.frame.XDispatchProviderInterception;
import com.sun.star.frame.XDispatchProviderInterceptor;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import complex.dispatches.Interceptor;
import java.util.HashMap;





// ---------- junit imports -----------------
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
// ------------------------------------------

//-----------------------------------------------
/** @short  Check the interface XDispatchInformationProvider

@descr  Because there exists more then one implementation of a dispatch
object, we have to test all these implementations ...
 */
public class checkdispatchapi
{
    //-------------------------------------------
    // some const

    //-------------------------------------------
    // member
    /** points to the global uno service manager. */
    private XMultiServiceFactory m_xMSF = null;
    private connectivity.tools.HsqlDatabase db;
    /** can be used to create new test frames. */
    private XFrame m_xDesktop = null;
    /** provides XDispatchInformationProvider interface. */
    private XFrame m_xFrame = null;

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
//                {
//                    "checkDispatchInfoOfWriter",
//                    "checkDispatchInfoOfCalc",
//                    "checkDispatchInfoOfDraw",
//                    "checkDispatchInfoOfImpress",
//                    "checkDispatchInfoOfMath",
//                    "checkDispatchInfoOfChart",
//                    "checkDispatchInfoOfBibliography",
//                    "checkDispatchInfoOfQueryDesign",
//                    "checkDispatchInfoOfTableDesign",
//                    "checkDispatchInfoOfFormGridView",
//                    "checkDispatchInfoOfDataSourceBrowser",
//                    "checkDispatchInfoOfRelationDesign",
//                    "checkDispatchInfoOfBasic",
//                    "checkDispatchInfoOfStartModule",
//                    "checkInterceptorLifeTime",
//                    "checkInterception"
//                };
//    }

    //-------------------------------------------
    /** @short  Create the environment for following tests.

    @descr  create an empty test frame, where we can load
    different components inside.
     */
    @Before public void before()
    {
        try
        {
            // get uno service manager from global test environment
            m_xMSF = getMSF();

            db = new connectivity.tools.HsqlDatabase(m_xMSF);

            // create desktop
            m_xDesktop = UnoRuntime.queryInterface(XFrame.class, m_xMSF.createInstance("com.sun.star.frame.Desktop"));

            m_xFrame = impl_createNewFrame();
        }
        catch (java.lang.Throwable ex)
        {
            fail("Cant initialize test environment.");
        }
    }

    //-------------------------------------------
    /** @short  close the environment.
     */
    @After public void after()
    {
        db.close();
        impl_closeFrame(m_xFrame);
        m_xFrame = null;
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfWriter()
    {
        impl_checkDispatchInfoOfXXX("private:factory/swriter");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfCalc()
    {
        impl_checkDispatchInfoOfXXX("private:factory/scalc");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfDraw()
    {
        impl_checkDispatchInfoOfXXX("private:factory/sdraw");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfImpress()
    {
        impl_checkDispatchInfoOfXXX("private:factory/simpress");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfChart()
    {
        impl_checkDispatchInfoOfXXX("private:factory/schart");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfMath()
    {
        impl_checkDispatchInfoOfXXX("private:factory/smath");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfDataBase()
    {
        impl_checkDispatchInfoOfXXX("private:factory/sdatabase");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfBibliography()
    {
        impl_checkDispatchInfoOfXXX(".component:Bibliography/View1");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfQueryDesign()
    {
        callDatabaseDispatch(".component:DB/QueryDesign");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfTableDesign()
    {
        callDatabaseDispatch(".component:DB/TableDesign");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfFormGridView()
    {
        impl_checkDispatchInfoOfXXX(".component:DB/FormGridView");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfDataSourceBrowser()
    {
        impl_checkDispatchInfoOfXXX(".component:DB/DataSourceBrowser");
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfRelationDesign()
    {
        callDatabaseDispatch(".component:DB/RelationDesign");
    }
    //-------------------------------------------

    private void callDatabaseDispatch(String url)
    {
        try
        {
            final PropertyValue args = new PropertyValue();
            args.Name = "ActiveConnection";
            args.Value = (Object) db.defaultConnection();

            XFrame xFrame = impl_createNewFrame();

            impl_loadIntoFrame(xFrame, url, new PropertyValue[]
                    {
                        args
                    });
            impl_checkDispatchInfo(xFrame);
            impl_closeFrame(xFrame);
        }
        catch (java.lang.Exception e)
        {
        }
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfBasic()
    {
        Object aComponent = impl_createUNOComponent("com.sun.star.script.BasicIDE");
        impl_checkDispatchInfo(aComponent);
    }

    //-------------------------------------------
    @Test public void checkDispatchInfoOfStartModule()
    {
        Object aComponent = impl_createUNOComponent("com.sun.star.frame.StartModule");
        impl_checkDispatchInfo(aComponent);
    }

    //-------------------------------------------
    public void checkInterceptorLifeTime()
    {
        // Note: It's important for the following test, that aInterceptor will be hold alive by the uno reference
        // xInterceptor. Otherwhise we cant check some internal states of aInterceptor at the end of this method, because
        // it was already killed .-)

        Interceptor aInterceptor = new Interceptor();
        XDispatchProviderInterceptor xInterceptor = UnoRuntime.queryInterface(XDispatchProviderInterceptor.class, aInterceptor);

        XFrame xFrame = impl_createNewFrame();
        XDispatchProviderInterception xInterception = UnoRuntime.queryInterface(XDispatchProviderInterception.class, xFrame);

        xInterception.registerDispatchProviderInterceptor(xInterceptor);
        impl_closeFrame(xFrame);

        int nRegCount = aInterceptor.getRegistrationCount();
        boolean bIsRegistered = aInterceptor.isRegistered();

        System.out.println("registration count = " + nRegCount);
        System.out.println("is registered ?    = " + bIsRegistered);

        if (nRegCount < 1)
        {
            fail("Interceptor was never registered.");
        }

        if (bIsRegistered)
        {
            fail("Interceptor was not deregistered automaticly on closing the corresponding frame.");
        }

        System.out.println("Destruction of interception chain works as designed .-)");
    }

    //-------------------------------------------
    public void checkInterception()
    {
        String[] lDisabledURLs = new String[1];
        lDisabledURLs[0] = ".uno:Open";

        System.out.println("create and initialize interceptor ...");
        Interceptor aInterceptor = new Interceptor();
        aInterceptor.setURLs4URLs4Blocking(lDisabledURLs);

        XDispatchProviderInterceptor xInterceptor = UnoRuntime.queryInterface(XDispatchProviderInterceptor.class, aInterceptor);

        System.out.println("create and initialize frame ...");
        XFrame xFrame = impl_createNewFrame();
        impl_loadIntoFrame(xFrame, "private:factory/swriter", null);

        XDispatchProviderInterception xInterception = UnoRuntime.queryInterface(XDispatchProviderInterception.class, xFrame);

        System.out.println("register interceptor ...");
        xInterception.registerDispatchProviderInterceptor(xInterceptor);

        System.out.println("deregister interceptor ...");
        xInterception.releaseDispatchProviderInterceptor(xInterceptor);
    }

    //-------------------------------------------
    private void impl_checkDispatchInfoOfXXX(String sXXX)
    {
        XFrame xFrame = impl_createNewFrame();
        impl_loadIntoFrame(xFrame, sXXX, null);
        impl_checkDispatchInfo(xFrame);
        impl_closeFrame(xFrame);
    }

    //-------------------------------------------
    /** @short  load an URL into the current test frame.
     */
    private void impl_loadIntoFrame(XFrame xFrame, String sURL, PropertyValue args[])
    {
        XComponentLoader xLoader = UnoRuntime.queryInterface(XComponentLoader.class, xFrame);
        if (xLoader == null)
        {
            fail("Frame does not provide required interface XComponentLoader.");
        }

        XComponent xDoc = null;
        try
        {
            xDoc = xLoader.loadComponentFromURL(sURL, "_self", 0, args);
        }
        catch (java.lang.Throwable ex)
        {
            xDoc = null;
        }

        if (xDoc == null)
        {
            fail("Could not load \"" + sURL + "\".");
        }
    }

    //-------------------------------------------
    /** @short  create an uno implementation directly.
     */
    private Object impl_createUNOComponent(String sName)
    {
        Object aComponent = null;
        try
        {
            aComponent = m_xMSF.createInstance(sName);
        }
        catch (java.lang.Throwable ex)
        {
            aComponent = null;
        }

        if (aComponent == null)
        {
            fail("Could not create UNO component \"" + sName + "\".");
        }
        return aComponent;
    }

    //-------------------------------------------
    /** @short  check the interface XDispatchInformationProvider
    at the specified component.
     */
    private void impl_checkDispatchInfo(Object aComponent)
    {
        XDispatchInformationProvider xInfoProvider = UnoRuntime.queryInterface(XDispatchInformationProvider.class, aComponent);
        if (xInfoProvider == null)
        {
            // Warning
            System.out.println("Warning:\tComponent does not provide the [optional!] interface XDispatchInformationProvider.");
            return;
        }

        try
        {
            short[] lGroups = xInfoProvider.getSupportedCommandGroups();
            int c1 = lGroups.length;
            int i1 = 0;
            for (i1 = 0; i1 < c1; ++i1)
            {
                short nGroup = lGroups[i1];
                DispatchInformation[] lInfos = xInfoProvider.getConfigurableDispatchInformation(nGroup);
                int c2 = lInfos.length;
                int i2 = 0;

                // check for empty lists
                // Warning
                if (lInfos.length < 1)
                {
                    System.out.println("Warning:\tCould not get any DispatchInformation for group [" + nGroup + "].");
                }

                // check for duplicates (and by the way, if the info item match the requested group)
                HashMap aCheckMap = new HashMap(c2);
                for (i2 = 0; i2 < c2; ++i2)
                {
                    DispatchInformation aInfo = lInfos[i2];
                    if (aInfo.GroupId != nGroup)
                    {
                        // Error
                        fail("At least one DispatchInformation item does not match the requested group.\n\trequested group=[" + nGroup
                                + "] returned groupd=[" + aInfo.GroupId + "] command=\"" + aInfo.Command + "\""); // true => dont break this test
                        continue;
                    }

                    if (aCheckMap.containsKey(aInfo.Command))
                    {
                        // Error
                        fail("Found a duplicate item: group=[" + aInfo.GroupId + "] command=\"" + aInfo.Command + "\""); // true => dont break this test
                        continue;
                    }

                    aCheckMap.put(aInfo.Command, aInfo.Command);
                    System.out.println("\t[" + aInfo.GroupId + "] \"" + aInfo.Command + "\"");
                }
            }
        }
        catch (java.lang.Throwable ex)
        {
            fail("Exception caught during using XDispatchInformationProvider.");
            // ex.printStackTrace();
        }
    }

    //-------------------------------------------
    private synchronized XFrame impl_createNewFrame()
    {
        XFrame xFrame = null;

        try
        {
            xFrame = m_xDesktop.findFrame("_blank", 0);
            xFrame.getContainerWindow().setVisible(true);
        }
        catch (java.lang.Throwable ex)
        {
            fail("Could not create the frame instance.");
        }

        return xFrame;
    }

    //-------------------------------------------
    private synchronized void impl_closeFrame(XFrame xFrame)
    {
        XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xFrame);
        try
        {
            xClose.close(false);
        }
        catch (com.sun.star.util.CloseVetoException exVeto)
        {
            fail("Test frame couldn't be closed successfully.");
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
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();
}
