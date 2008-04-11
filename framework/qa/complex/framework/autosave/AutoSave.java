/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AutoSave.java,v $
 * $Revision: 1.4 $
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

package complex.framework.autosave;

import com.sun.star.frame.*;
import com.sun.star.lang.*;
import com.sun.star.util.*;
import com.sun.star.beans.*;
import com.sun.star.uno.*;
import com.sun.star.sheet.*;
import com.sun.star.table.*;

import java.util.*;

import complexlib.*;
import helper.*;
import util.*;

//-----------------------------------------------
/** @short  Check some use cases of the AutoSave feature
 */
public class AutoSave extends ComplexTestCase
{
    //-------------------------------------------
    class AutoSaveListener implements XStatusListener
    {
        private XDispatch m_xAutoSave;
        private URL m_aRegistration;
        private Protocol m_aLog;

        public AutoSaveListener(XMultiServiceFactory xSMGR    ,
                                XDispatch            xAutoSave,
                                Protocol             aLog     )
        {
            m_aLog = aLog;
            m_aLog.log(Protocol.TYPE_SCOPE_OPEN, "create listener for AutoSave notifications ...");

            try
            {
                m_xAutoSave = xAutoSave;

                XURLTransformer xParser = (XURLTransformer)UnoRuntime.queryInterface(
                                                XURLTransformer.class,
                                                xSMGR.createInstance("com.sun.star.util.URLTransformer"));
                URL[] aURL = new URL[1];
                aURL[0] = new URL();
                aURL[0].Complete = "vnd.sun.star.autorecovery:/doAutoSave";
                xParser.parseStrict(aURL);
                m_aRegistration = aURL[0];

                m_xAutoSave.addStatusListener(this, m_aRegistration);
                m_aLog.log(Protocol.TYPE_INFO, "successfully registered as AutoSave listener.");
            }
            catch(Throwable ex)
            {
                m_aLog.log(ex);
            }

            m_aLog.log(Protocol.TYPE_SCOPE_CLOSE, "");
        }

        public void disableListener()
        {
            m_aLog.log(Protocol.TYPE_SCOPE_OPEN, "stop listening for AutoSave notifications ...");

            XDispatch xAutoSave = null;
            URL       aRegURL   = null;
            synchronized (this)
            {
                xAutoSave = m_xAutoSave;
                aRegURL   = m_aRegistration;
            }

            try
            {
                if (
                    (xAutoSave != null) &&
                    (aRegURL   != null)
                   )
                    xAutoSave.removeStatusListener(this, aRegURL);
            }
            catch(Throwable ex)
            {
                m_aLog.log(ex);
            }

            m_aLog.log(Protocol.TYPE_SCOPE_CLOSE, "");
        }

        public void statusChanged(FeatureStateEvent aEvent)
        {
            m_aLog.log(Protocol.TYPE_SCOPE_OPEN, "statusChanged() called from AutoSave ...");

            m_aLog.log("FeatureURL        = \""+aEvent.FeatureURL.Complete+"\"" );
            m_aLog.log("FeatureDescriptor = \""+aEvent.FeatureDescriptor+"\""   );
            m_aLog.log("IsEnabled         = \""+aEvent.IsEnabled+"\""           );
            m_aLog.log("Requery           = \""+aEvent.Requery+"\""             );
            m_aLog.log("State:"                                                 );
            m_aLog.log(aEvent.State                                             );

            m_aLog.log(Protocol.TYPE_SCOPE_CLOSE, "");
        }

        public void disposing(com.sun.star.lang.EventObject aEvent)
        {
            m_aLog.log(Protocol.TYPE_INFO, "disposing() called from AutoSave.");
            synchronized(this)
            {
                m_xAutoSave     = null;
                m_aRegistration = null;
            }
        }
    }

    //-------------------------------------------
    // some const

    //-------------------------------------------
    // member

    private Protocol m_aLog;

    /** points to the global uno service manager. */
    private XMultiServiceFactory m_xSMGR = null;

    private SOfficeFactory m_aSOF;

    /** can be used to trigger/enable/disable the AutoSave feature. */
    private XDispatch m_xAutoSave = null;

    /** a test document, which needs some time for saving to simulate concurrent
     *  save operations. */
    private XStorable m_xTestDoc = null;

    private XURLTransformer m_xURLParser = null;

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
            "checkConcurrentAutoSaveToNormalUISave",
        };
    }

    //-------------------------------------------
    /** @short  Create the environment for following tests.

        @descr  create an empty test frame, where we can load
                different components inside.
     */
    public void before()
    {
        m_aLog = new Protocol(Protocol.MODE_HTML | Protocol.MODE_STDOUT, Protocol.FILTER_NONE, utils.getUsersTempDir() + "/complex_log_ascii_01.html");

        try
        {
            // get uno service manager from global test environment
            m_xSMGR = (XMultiServiceFactory)param.getMSF();

            // get another helper to e.g. create test documents
            m_aSOF = SOfficeFactory.getFactory(m_xSMGR);

            // create AutoSave instance
            m_xAutoSave = (XDispatch)UnoRuntime.queryInterface(
                            XDispatch.class,
                            m_xSMGR.createInstance("com.sun.star.comp.framework.AutoRecovery"));

            // prepare AutoSave
            // make sure it will be started every 1 min
            ConfigHelper aConfig = new ConfigHelper(m_xSMGR, "org.openoffice.Office.Recovery", false);
            aConfig.writeRelativeKey("AutoSave", "Enabled"      , Boolean.TRUE  );
            aConfig.writeRelativeKey("AutoSave", "TimeIntervall", new Integer(1)); // 1 min
            aConfig.flush();
            aConfig = null;

            // is needed to parse dispatch commands
            m_xURLParser = (XURLTransformer)UnoRuntime.queryInterface(
                                            XURLTransformer.class,
                                            m_xSMGR.createInstance("com.sun.star.util.URLTransformer"));

        }
        catch(java.lang.Throwable ex)
        {
            m_aLog.log(ex);
            failed("Couldn't create test environment");
        }
    }

    //-------------------------------------------
    /** @short  close the environment.
     */
    public void after()
    {
        // ???
    }

    //-------------------------------------------
    // create a calc document with content, which needs some time for saving
    private XInterface createBigCalcDoc()
    {
        m_aLog.log(Protocol.TYPE_SCOPE_OPEN, "createBigCalcDoc() started ...");
        try
        {
            m_aLog.log("Create empty calc document for testing.");
            XSpreadsheetDocument xSheetDoc   = m_aSOF.createCalcDoc("_default");
            m_aLog.log("Retrieve first sheet from calc document.");
            XSpreadsheets        xSheets     = xSheetDoc.getSheets();
            XSpreadsheet         xSheet      = (XSpreadsheet)AnyConverter.toObject(
                                                 new Type(XSpreadsheet.class),
                                                 xSheets.getByName(
                                                         xSheets.getElementNames()[0]));
            m_aLog.log("Fill two cells with value and formula.");
            xSheet.getCellByPosition(0, 0).setValue(1);
            xSheet.getCellByPosition(0, 1).setFormula("=a1+1");
            m_aLog.log("Retrieve big range.");
            XCellRange           xRange      = xSheet.getCellRangeByName("A1:Z9999");
            XCellSeries          xSeries     = (XCellSeries)UnoRuntime.queryInterface(
                                                     XCellSeries.class,
                                                     xRange);
            m_aLog.log("Duplicate cells from top to bottom inside range.");
            xSeries.fillAuto(FillDirection.TO_BOTTOM, 2);
            m_aLog.log("Duplicate cells from left to right inside range.");
            xSeries.fillAuto(FillDirection.TO_RIGHT , 1);

            m_aLog.log(Protocol.TYPE_SCOPE_CLOSE | Protocol.TYPE_OK, "createBigCalcDoc() finished.");
            return xSheetDoc;
        }
        catch(Throwable ex)
        {
            m_aLog.log(ex);
        }

        m_aLog.log(Protocol.TYPE_SCOPE_CLOSE, "createBigCalcDoc() finished.");
        return null;
    }

    //-------------------------------------------
    private void saveDoc(XInterface xDoc,
                         String     sURL)
    {
        m_aLog.log(Protocol.TYPE_SCOPE_OPEN, "saveDoc('"+sURL+"') started ...");
        try
        {
            URL[] aURL       = new URL[1];
            aURL[0]          = new URL();
            aURL[0].Complete = ".uno:SaveAs";
            m_xURLParser.parseStrict(aURL);

            XModel xModel = (XModel)UnoRuntime.queryInterface(
                                XModel.class,
                                xDoc);
            XDispatchProvider xProvider = (XDispatchProvider)UnoRuntime.queryInterface(
                                                XDispatchProvider.class,
                                                xModel.getCurrentController());
            XDispatch xDispatch = xProvider.queryDispatch(aURL[0], "_self", 0);

            PropertyValue[] lArgs = new PropertyValue[3];
            lArgs[0] = new PropertyValue();
            lArgs[0].Name  = "URL";
            lArgs[0].Value = sURL;
            lArgs[1] = new PropertyValue();
            lArgs[1].Name  = "Overwrite";
            lArgs[1].Value = Boolean.TRUE;
            lArgs[2] = new PropertyValue();
            lArgs[2].Name  = "StoreTo";
            lArgs[2].Value = Boolean.TRUE;

            xDispatch.dispatch(aURL[0], lArgs);

            m_aLog.log(Protocol.TYPE_OK, "saveDoc('"+sURL+"') = OK.");
        }
/*
        catch(com.sun.star.io.IOException exIO)
        {
            m_aLog.log(Protocol.TYPE_WARNING     , "got IOException on calling doc.store()."                                                            );
            m_aLog.log(Protocol.TYPE_WARNING_INFO, "Please check the reason for that more in detail."                                                   );
            m_aLog.log(Protocol.TYPE_WARNING_INFO, "A message like \"Concurrent save requests are not allowed.\" was intended and doesnt show an error!");
            m_aLog.log(Protocol.TYPE_WARNING_INFO, "Message of the original exception:"                                                                 );
            m_aLog.log(Protocol.TYPE_WARNING_INFO, exIO.getMessage());
        }
*/
        catch(Throwable ex)
        {
            m_aLog.log(ex);
        }
        m_aLog.log(Protocol.TYPE_SCOPE_CLOSE, "saveDoc('"+sURL+"') finished.");
    }

    //-------------------------------------------
    private void closeDoc(XInterface xDoc)
    {
        m_aLog.log(Protocol.TYPE_SCOPE_OPEN, "closeDoc() started ...");

        try
        {
            Random aRandom = new Random();
            int    nRetry  = 5;
            while(nRetry>0)
            {
                try
                {
                    XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(
                                            XCloseable.class,
                                            xDoc);
                    if (xClose != null)
                    {
                        xClose.close(false);
                        m_aLog.log(Protocol.TYPE_OK, "closeDoc() = OK.");
                        nRetry = 0;
                    }
                    else
                        m_aLog.log(Protocol.TYPE_ERROR, "closeDoc() = ERROR. Doc doesnt provide needed interface!");
                }
                catch(com.sun.star.util.CloseVetoException exVeto)
                {
                    m_aLog.log(Protocol.TYPE_WARNING     , "got CloseVetoException on calling doc.close()."                                    );
                    m_aLog.log(Protocol.TYPE_WARNING_INFO, "Please check the reason for that more in detail."                                  );
                    m_aLog.log(Protocol.TYPE_WARNING_INFO, "A message like \"Cant close while saving.\" was intended and doesnt show an error!");
                    m_aLog.log(Protocol.TYPE_WARNING_INFO, exVeto.getMessage());
                }

                if (nRetry > 0)
                {
                    --nRetry;
                    long nWait = (long)aRandom.nextInt(30000); // 30 sec.
                    try
                    {
                        m_aLog.log(Protocol.TYPE_INFO, "sleep for "+nWait+" ms");
                        synchronized(this)
                        {
                            wait(nWait);
                        }
                    }
                    catch(Throwable ex)
                    {
                        m_aLog.log(Protocol.TYPE_WARNING     , "got exception for wait() !?");
                        m_aLog.log(Protocol.TYPE_WARNING_INFO, ex.getMessage());
                    }
                }
            }
        }
        catch(Throwable ex)
        {
            m_aLog.log(ex);
        }

        m_aLog.log(Protocol.TYPE_SCOPE_CLOSE, "closeDoc() finished.");
    }

    class DocThread extends Thread
    {
        DocThread()
        {}

        public void run()
        {
            impl_checkConcurrentAutoSaveToNormalUISave();
        }
    }

    //-------------------------------------------
    /** @short  check concurrent save requests to the same document
     *          at the same time.
     *
     *  @descr  First we simulate an UI save by dispatching the right URL
     *          to the document and at the same time we try to trigger an AutoSave
     *          from another thread. So these operations should be started at the same time.
     *          It should not crash. The AutoSave request must be postphoned.
     */
    public void checkConcurrentAutoSaveToNormalUISave()
    {
        m_aLog.log(Protocol.TYPE_TESTMARK , "AutoSave");
        m_aLog.log(Protocol.TYPE_SCOPE_OPEN, "checkConcurrentAutoSaveToNormalUISave()");

        AutoSaveListener xListener = new AutoSaveListener(m_xSMGR, m_xAutoSave, m_aLog);

        try
        {
            DocThread aThread = new DocThread();
            aThread.start();
            aThread.join();
        }
        catch(Throwable ex)
        {}

        xListener.disableListener();

        m_aLog.log(Protocol.TYPE_SCOPE_CLOSE, "checkConcurrentAutoSaveToNormalUISave()");
        m_aLog.logStatistics();
    }

    public void impl_checkConcurrentAutoSaveToNormalUISave()
    {
        Random aRandom = new Random();

        int i = 0;
        int c = 5;
        for (i=0; i<c; ++i)
        {
            XInterface xDoc = createBigCalcDoc();
            try
            {
                long nWait = (long)aRandom.nextInt(120000);
                m_aLog.log(Protocol.TYPE_INFO, "sleep for "+nWait+" ms");
                synchronized(this)
                {
                    wait(nWait);
                }
            }
            catch(Throwable ex)
            {
                m_aLog.log(Protocol.TYPE_WARNING     , "got exception for wait() !?");
                m_aLog.log(Protocol.TYPE_WARNING_INFO, ex.getMessage());
            }
            saveDoc(xDoc, utils.getOfficeTemp(m_xSMGR) + "/test_calc.ods");
            closeDoc(xDoc);
        }
    }
}
