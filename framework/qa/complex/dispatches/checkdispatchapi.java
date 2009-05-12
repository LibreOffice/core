/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: checkdispatchapi.java,v $
 * $Revision: 1.5 $
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

import com.sun.star.frame.*;
import com.sun.star.lang.*;
import com.sun.star.util.*;
import com.sun.star.beans.*;
import com.sun.star.uno.*;

import java.util.*;

import complexlib.ComplexTestCase;

import helper.*;

//-----------------------------------------------
/** @short  Check the interface XDispatchInformationProvider

    @descr  Because there exists more then one implementation of a dispatch
            object, we have to test all these implementations ...
 */
public class checkdispatchapi extends ComplexTestCase
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
    public String[] getTestMethodNames()
    {
        return new String[]
        {
            "checkDispatchInfoOfWriter",
            "checkDispatchInfoOfCalc",
            "checkDispatchInfoOfDraw",
            "checkDispatchInfoOfImpress",
            "checkDispatchInfoOfMath",
            "checkDispatchInfoOfChart",
            "checkDispatchInfoOfBibliography",
            "checkDispatchInfoOfQueryDesign",
            "checkDispatchInfoOfTableDesign",
            "checkDispatchInfoOfFormGridView",
            "checkDispatchInfoOfDataSourceBrowser",
            "checkDispatchInfoOfRelationDesign",
            "checkDispatchInfoOfBasic",
            "checkDispatchInfoOfStartModule",
            "checkInterceptorLifeTime",
            "checkInterception"
        };
    }

    //-------------------------------------------
    /** @short  Create the environment for following tests.

        @descr  create an empty test frame, where we can load
                different components inside.
     */
    public void before()
    {
        try
        {
            // get uno service manager from global test environment
            m_xMSF = (XMultiServiceFactory)param.getMSF();

            db = new connectivity.tools.HsqlDatabase(m_xMSF);

            // create desktop
            m_xDesktop = (XFrame)UnoRuntime.queryInterface(
                                XFrame.class,
                                m_xMSF.createInstance("com.sun.star.frame.Desktop"));

            m_xFrame = impl_createNewFrame();
        }
        catch(java.lang.Throwable ex)
        {
            failed("Cant initialize test environment.");
        }
    }

    //-------------------------------------------
    /** @short  close the environment.
     */
    public void after()
    {
        db.close();
        impl_closeFrame(m_xFrame);
        m_xFrame = null;
    }

    //-------------------------------------------
    public void checkDispatchInfoOfWriter()
    {
        impl_checkDispatchInfoOfXXX("private:factory/swriter");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfCalc()
    {
        impl_checkDispatchInfoOfXXX("private:factory/scalc");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfDraw()
    {
        impl_checkDispatchInfoOfXXX("private:factory/sdraw");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfImpress()
    {
        impl_checkDispatchInfoOfXXX("private:factory/simpress");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfChart()
    {
        impl_checkDispatchInfoOfXXX("private:factory/schart");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfMath()
    {
        impl_checkDispatchInfoOfXXX("private:factory/smath");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfDataBase()
    {
        impl_checkDispatchInfoOfXXX("private:factory/sdatabase");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfBibliography()
    {
        impl_checkDispatchInfoOfXXX(".component:Bibliography/View1");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfQueryDesign()
    {
        callDatabaseDispatch(".component:DB/QueryDesign");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfTableDesign()
    {
        callDatabaseDispatch(".component:DB/TableDesign");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfFormGridView()
    {
        impl_checkDispatchInfoOfXXX(".component:DB/FormGridView");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfDataSourceBrowser()
    {
        impl_checkDispatchInfoOfXXX(".component:DB/DataSourceBrowser");
    }

    //-------------------------------------------
    public void checkDispatchInfoOfRelationDesign()
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
            args.Value = (Object)db.defaultConnection();

            XFrame xFrame = impl_createNewFrame();

            impl_loadIntoFrame(xFrame, url, new PropertyValue[] { args });
            impl_checkDispatchInfo(xFrame);
            impl_closeFrame(xFrame);
         } catch(java.lang.Exception e ) {
         }
    }

    //-------------------------------------------
    public void checkDispatchInfoOfBasic()
    {
        Object aComponent = impl_createUNOComponent("com.sun.star.script.BasicIDE");
        impl_checkDispatchInfo(aComponent);
    }

    //-------------------------------------------
    public void checkDispatchInfoOfStartModule()
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

        Interceptor aInterceptor = new Interceptor(log);
        com.sun.star.frame.XDispatchProviderInterceptor xInterceptor = (com.sun.star.frame.XDispatchProviderInterceptor)UnoRuntime.queryInterface(
                                                                                com.sun.star.frame.XDispatchProviderInterceptor.class,
                                                                                aInterceptor);

        com.sun.star.frame.XFrame xFrame = impl_createNewFrame();
        com.sun.star.frame.XDispatchProviderInterception xInterception = (com.sun.star.frame.XDispatchProviderInterception)UnoRuntime.queryInterface(
                                                                                com.sun.star.frame.XDispatchProviderInterception.class,
                                                                                xFrame);

        xInterception.registerDispatchProviderInterceptor(xInterceptor);
        impl_closeFrame(xFrame);

        int     nRegCount     = aInterceptor.getRegistrationCount();
        boolean bIsRegistered = aInterceptor.isRegistered();

        log.println("registration count = "+nRegCount    );
        log.println("is registered ?    = "+bIsRegistered);

        if (nRegCount < 1)
            failed("Interceptor was never registered.");

        if (bIsRegistered)
            failed("Interceptor was not deregistered automaticly on closing the corresponding frame.");

        log.println("Destruction of interception chain works as designed .-)");
    }

    //-------------------------------------------
    public void checkInterception()
    {
        String [] lDisabledURLs    = new String [1];
                  lDisabledURLs[0] = ".uno:Open";

        log.println("create and initialize interceptor ...");
        Interceptor aInterceptor = new Interceptor(log);
        aInterceptor.setURLs4URLs4Blocking(lDisabledURLs);

        com.sun.star.frame.XDispatchProviderInterceptor xInterceptor = (com.sun.star.frame.XDispatchProviderInterceptor)UnoRuntime.queryInterface(
                                                                                com.sun.star.frame.XDispatchProviderInterceptor.class,
                                                                                aInterceptor);

        log.println("create and initialize frame ...");
        com.sun.star.frame.XFrame xFrame = impl_createNewFrame();
        impl_loadIntoFrame(xFrame, "private:factory/swriter", null);

        com.sun.star.frame.XDispatchProviderInterception xInterception = (com.sun.star.frame.XDispatchProviderInterception)UnoRuntime.queryInterface(
                                                                                com.sun.star.frame.XDispatchProviderInterception.class,
                                                                                xFrame);

        log.println("register interceptor ...");
        xInterception.registerDispatchProviderInterceptor(xInterceptor);

        log.println("deregister interceptor ...");
        xInterception.releaseDispatchProviderInterceptor(xInterceptor);
    }

    //-------------------------------------------
    private void impl_checkDispatchInfoOfXXX(String sXXX)
    {
        XFrame xFrame = impl_createNewFrame();
        impl_loadIntoFrame(xFrame, sXXX,null);
        impl_checkDispatchInfo(xFrame);
        impl_closeFrame(xFrame);
    }

    //-------------------------------------------
    /** @short  load an URL into the current test frame.
     */
    private void impl_loadIntoFrame(XFrame xFrame, String sURL,PropertyValue args[])
    {
        XComponentLoader xLoader = (XComponentLoader)UnoRuntime.queryInterface(
                                        XComponentLoader.class,
                                        xFrame);
        if (xLoader == null)
            failed("Frame does not provide required interface XComponentLoader.");

        XComponent xDoc = null;
        try
        {
            xDoc = xLoader.loadComponentFromURL(sURL, "_self", 0, args);
        }
        catch(java.lang.Throwable ex)
        {
            xDoc = null;
        }

        if (xDoc == null)
            failed("Could not load \""+sURL+"\".");
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
        catch(java.lang.Throwable ex)
        {
            aComponent = null;
        }

        if (aComponent == null)
            failed("Could not create UNO component \""+sName+"\".");
        return aComponent;
    }

    //-------------------------------------------
    /** @short  check the interface XDispatchInformationProvider
                at the specified component.
     */
    private void impl_checkDispatchInfo(Object aComponent)
    {
        XDispatchInformationProvider xInfoProvider = (XDispatchInformationProvider)UnoRuntime.queryInterface(
                                                        XDispatchInformationProvider.class,
                                                        aComponent);
        if (xInfoProvider == null)
        {
            // Warning
            log.println("Warning:\tComponent does not provide the [optional!] interface XDispatchInformationProvider.");
            return;
        }

        try
        {
            short[] lGroups = xInfoProvider.getSupportedCommandGroups();
            int     c1      = lGroups.length;
            int     i1      = 0;
            for (i1=0; i1<c1; ++i1)
            {
                short                 nGroup = lGroups[i1];
                DispatchInformation[] lInfos = xInfoProvider.getConfigurableDispatchInformation(nGroup);
                int                   c2     = lInfos.length;
                int                   i2     = 0;

                // check for empty lists
                // Warning
                if (lInfos.length < 1)
                    log.println("Warning:\tCould not get any DispatchInformation for group ["+nGroup+"].");

                // check for duplicates (and by the way, if the info item match the requested group)
                HashMap aCheckMap = new HashMap(c2);
                for (i2=0; i2<c2; ++i2)
                {
                    DispatchInformation aInfo = lInfos[i2];
                    if (aInfo.GroupId != nGroup)
                    {
                        // Error
                        failed("At least one DispatchInformation item does not match the requested group.\n\trequested group=["+nGroup+
                               "] returned groupd=["+aInfo.GroupId+"] command=\""+aInfo.Command+"\"", true); // true => dont break this test
                        continue;
                    }

                    if (aCheckMap.containsKey(aInfo.Command))
                    {
                        // Error
                        failed("Found a duplicate item: group=["+aInfo.GroupId+"] command=\""+aInfo.Command+"\"", true); // true => dont break this test
                        continue;
                    }

                    aCheckMap.put(aInfo.Command, aInfo.Command);
                    log.println("\t["+aInfo.GroupId+"] \""+aInfo.Command+"\"");
                }
            }
        }
        catch(java.lang.Throwable ex)
        {
            failed("Exception caught during using XDispatchInformationProvider.");
            ex.printStackTrace();
        }
    }

    //-------------------------------------------
    private synchronized com.sun.star.frame.XFrame impl_createNewFrame()
    {
        com.sun.star.frame.XFrame xFrame = null;

        try
        {
            xFrame = m_xDesktop.findFrame("_blank", 0);
            xFrame.getContainerWindow().setVisible(true);
        }
        catch(java.lang.Throwable ex)
        {
            failed("Could not create the frame instance.");
        }

        return xFrame;
   }

    //-------------------------------------------
    private synchronized void impl_closeFrame(com.sun.star.frame.XFrame xFrame)
    {
        com.sun.star.util.XCloseable xClose = (com.sun.star.util.XCloseable)UnoRuntime.queryInterface(
                                                    com.sun.star.util.XCloseable.class,
                                                    xFrame);
        try
        {
            xClose.close(false);
        }
        catch(com.sun.star.util.CloseVetoException exVeto)
        {
            failed("Test frame couldn't be closed successfully.");
        }
    }
}
