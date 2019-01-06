/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
import com.sun.star.awt.XReschedule;
import com.sun.star.awt.XToolkitExperimental;
import java.util.HashMap;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/** @short  Check the interface XDispatchInformationProvider

@descr  Because there exists more than one implementation of a dispatch
object, we have to test all these implementations ...
 */
public class checkdispatchapi
{
    // member
    /** points to the global uno service manager. */
    private XMultiServiceFactory m_xMSF = null;

//    private connectivity.tools.HsqlDatabase db;

    /** can be used to create new test frames. */
    private XFrame m_xDesktop = null;
    /** provides XDispatchInformationProvider interface. */
    private XFrame m_xFrame = null;


    // test environment

    /** @short  Create the environment for following tests.

    @descr  create an empty test frame, where we can load
    different components inside.
     */
    @Before public void before() throws Exception
    {
        // get uno service manager from global test environment
        m_xMSF = getMSF();

//        db = new connectivity.tools.HsqlDatabase(m_xMSF);

        // create desktop
        m_xDesktop = UnoRuntime.queryInterface(XFrame.class, m_xMSF.createInstance("com.sun.star.frame.Desktop"));

        m_xFrame = impl_createNewFrame();
    }

    /** @short  close the environment.
     */
    @After public void after() throws Exception
    {
//        db.close();
        impl_closeFrame(m_xFrame);
        m_xFrame = null;
    }

    @Test public void checkDispatchInfoOfWriter() throws Exception
    {
        impl_checkDispatchInfoOfXXX("private:factory/swriter");
    }

    @Test public void checkDispatchInfoOfCalc() throws Exception
    {
        impl_checkDispatchInfoOfXXX("private:factory/scalc");
    }

    @Test public void checkDispatchInfoOfDraw() throws Exception
    {
        impl_checkDispatchInfoOfXXX("private:factory/sdraw");
    }

    @Test public void checkDispatchInfoOfImpress() throws Exception
    {
        impl_checkDispatchInfoOfXXX("private:factory/simpress");
    }

    @Test public void checkDispatchInfoOfChart() throws Exception
    {
        impl_checkDispatchInfoOfXXX("private:factory/schart");
    }

    @Test public void checkDispatchInfoOfMath() throws Exception
    {
        impl_checkDispatchInfoOfXXX("private:factory/smath");
    }

    @Test public void checkDispatchInfoOfDataBase() throws Exception
    {
//        impl_checkDispatchInfoOfXXX("private:factory/sdatabase");
    }

    @Test public void checkDispatchInfoOfBibliography() throws Exception
    {
//        impl_checkDispatchInfoOfXXX(".component:Bibliography/View1");
    }

    @Test public void checkDispatchInfoOfQueryDesign()
    {
//        callDatabaseDispatch(".component:DB/QueryDesign");
    }

    @Test public void checkDispatchInfoOfTableDesign() throws Exception
    {
//        callDatabaseDispatch(".component:DB/TableDesign");
    }

    @Test public void checkDispatchInfoOfFormGridView() throws Exception
    {
//        impl_checkDispatchInfoOfXXX(".component:DB/FormGridView");
    }

    @Test public void checkDispatchInfoOfDataSourceBrowser() throws Exception
    {
//        impl_checkDispatchInfoOfXXX(".component:DB/DataSourceBrowser");
    }

    @Test public void checkDispatchInfoOfRelationDesign()
    {
//        callDatabaseDispatch(".component:DB/RelationDesign");
    }

    private void callDatabaseDispatch(String url)
    {
/*        disabled along with all db related tests for now.

        try
        {
            final PropertyValue args = new PropertyValue();
            args.Name = "ActiveConnection";
            args.Value = db.defaultConnection();

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
        } */
    }

    @Test public void checkDispatchInfoOfBasic() throws Exception
    {
        Object aComponent = impl_createUNOComponent("com.sun.star.script.BasicIDE");
        impl_checkDispatchInfo(aComponent);
    }

    @Test public void checkDispatchInfoOfStartModule() throws Exception
    {
        Object aComponent = impl_createUNOComponent("com.sun.star.frame.StartModule");
        impl_checkDispatchInfo(aComponent);
    }

    @Test public void checkInterceptorLifeTime() throws Exception
    {
        // Note: It's important for the following test, that aInterceptor will be hold alive by the uno reference
        // xInterceptor. Otherwise we can't check some internal states of aInterceptor at the end of this method, because
        // it was already killed .-)

        Interceptor aInterceptor = new Interceptor(m_xMSF);
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
            fail("Interceptor was not deregistered automatically on closing the corresponding frame.");
        }

        System.out.println("Destruction of interception chain works as designed .-)");
    }

    @Test public void checkInterception() throws Exception
    {
        String[] lDisabledURLs = new String[] { ".uno:Open" };

        System.out.println("create and initialize interceptor ...");
        Interceptor aInterceptor = new Interceptor(m_xMSF);
        aInterceptor.setURLs4URLs4Blocking(lDisabledURLs);

        XDispatchProviderInterceptor xInterceptor = UnoRuntime.queryInterface(XDispatchProviderInterceptor.class, aInterceptor);

        System.out.println("create and initialize frame ...");
        XFrame xFrame = impl_createNewFrame();

        XDispatchProviderInterception xInterception = UnoRuntime.queryInterface(XDispatchProviderInterception.class, xFrame);
        System.out.println("register interceptor ...");
        xInterception.registerDispatchProviderInterceptor(xInterceptor);

        impl_loadIntoFrame(xFrame, "private:factory/swriter", null);

        // Framework dispatcher update is on a ~50ms wait.
        Thread.sleep(100);

        XReschedule m_xReschedule = UnoRuntime.queryInterface(
            XReschedule.class, m_xMSF.createInstance("com.sun.star.awt.Toolkit"));
        // queryDispatch for toolbars etc. happens asynchronously.
        System.out.println("process deferred events ...");
        m_xReschedule.reschedule();
        XToolkitExperimental m_xIdles = UnoRuntime.queryInterface(
            XToolkitExperimental.class, m_xReschedule);
        m_xIdles.processEventsToIdle();

        System.out.println("deregister interceptor ...");
        xInterception.releaseDispatchProviderInterceptor(xInterceptor);
    }

    private void impl_checkDispatchInfoOfXXX(String sXXX) throws Exception
    {
        XFrame xFrame = impl_createNewFrame();
        impl_loadIntoFrame(xFrame, sXXX, null);
        impl_checkDispatchInfo(xFrame);
        impl_closeFrame(xFrame);
    }

    /** @short  load an URL into the current test frame.
     */
    private void impl_loadIntoFrame(XFrame xFrame, String sURL, PropertyValue args[]) throws Exception
    {
        XComponentLoader xLoader = UnoRuntime.queryInterface(XComponentLoader.class, xFrame);
        if (xLoader == null)
        {
            fail("Frame does not provide required interface XComponentLoader.");
        }

        XComponent xDoc = xLoader.loadComponentFromURL(sURL, "_self", 0, args);

        if (xDoc == null)
        {
            fail("Could not load \"" + sURL + "\".");
        }
    }

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

    /** @short  check the interface XDispatchInformationProvider
    at the specified component.
     */
    private void impl_checkDispatchInfo(Object aComponent) throws Exception
    {
        XDispatchInformationProvider xInfoProvider = UnoRuntime.queryInterface(XDispatchInformationProvider.class, aComponent);
        if (xInfoProvider == null)
        {
            // Warning
            System.out.println("Warning:\tComponent does not provide the [optional!] interface XDispatchInformationProvider.");
            return;
        }

        short[] lGroups = xInfoProvider.getSupportedCommandGroups();
        int c1 = lGroups.length;
        int i1 = 0;
        for (i1 = 0; i1 < c1; ++i1)
        {
            short nGroup = lGroups[i1];
            DispatchInformation[] lInfos = xInfoProvider.getConfigurableDispatchInformation(nGroup, 0);
            int c2 = lInfos.length;
            int i2 = 0;

            // check for empty lists
            // Warning
            if (lInfos.length < 1)
            {
                System.out.println("Warning:\tCould not get any DispatchInformation for group [" + nGroup + "].");
            }

            // check for duplicates (and by the way, if the info item match the requested group)
            HashMap<String, String> aCheckMap = new HashMap<String, String>(c2);
            for (i2 = 0; i2 < c2; ++i2)
            {
                DispatchInformation aInfo = lInfos[i2];
                if (aInfo.GroupId != nGroup)
                {
                    // Error
                    fail("At least one DispatchInformation item does not match the requested group.\n\trequested group=[" + nGroup
                            + "] returned group=[" + aInfo.GroupId + "] command=\"" + aInfo.Command + "\""); // true => don't break this test
                    continue;
                }

                if (aCheckMap.containsKey(aInfo.Command))
                {
                    // Error
                    fail("Found a duplicate item: group=[" + aInfo.GroupId + "] command=\"" + aInfo.Command + "\""); // true => don't break this test
                    continue;
                }

                aCheckMap.put(aInfo.Command, aInfo.Command);
                System.out.println("\t[" + aInfo.GroupId + "] \"" + aInfo.Command + "\"");
            }
        }
    }

    private synchronized XFrame impl_createNewFrame()
    {
        XFrame xFrame = m_xDesktop.findFrame("_blank", 0);
        xFrame.getContainerWindow().setVisible(true);
        return xFrame;
    }

    private synchronized void impl_closeFrame(XFrame xFrame) throws Exception
    {
        XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xFrame);
        xClose.close(false);
    }

    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
