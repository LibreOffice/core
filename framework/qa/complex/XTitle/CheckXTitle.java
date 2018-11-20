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

package complex.XTitle;

import static org.junit.Assert.*;
import helper.URLHelper;
import util.utils;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import org.openoffice.test.OfficeFileUrl;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.Desktop;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFrame2;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XTitle;
import com.sun.star.frame.XController;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URLTransformer;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XURLTransformer;
import com.sun.star.util.URL;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XDispatch;

/** @short Check the interface XTitle

    @descr These tests check in various cases the correctness of
           the title in the ui window.
*/

public class CheckXTitle
{

    // some const
    private static final String DOCUMENT_TITLE                    = "documentTitle";
    private static final String UNO_URL_FOR_PRINT_PREVIEW         = ".uno:PrintPreview";
    private static final String UNO_URL_FOR_CLOSING_PRINT_PREVIEW = ".uno:ClosePreview";
    private static final String UNO_URL_FOR_CLOSING_DOC           = ".uno:CloseWin";
    private static final String UNO_URL_FOR_READ_ONLY             = ".uno:OpenReadOnly";

    // members

    /** will be set to xDesktop OR xFrame. */
    private XComponentLoader m_xLoader = null;

    /** provides uno service manager */
    private XMultiServiceFactory m_xMSF = null;

    /** used for parsing uno query URLs */
    private XURLTransformer m_xParser = null;

    // test environment
    /** @short  Create the environment for following tests.

        @descr  Use either a component loader from desktop or
                from frame
     */
    @Before public void before() throws Exception
    {
        // get uno service manager from global test environment
        /* points to the global uno service manager. */
        m_xMSF = getMSF();

        // define default loader
        m_xLoader = Desktop.create(connection.getComponentContext());

        // get URL parser
        m_xParser = URLTransformer.create(connection.getComponentContext());
    }

    /** @short  close the environment.
     */
    @After public void after() throws Exception
    {
        m_xLoader = null;
        m_xParser = null;
        m_xMSF    = null;
    }

    // prepare an uno URL query and dispatch it
    private void prepareQueryAndDispatch(XDispatchProvider xDisProv, String unoURL)
    {
        XDispatch xDispatcher = null;
        URL[] aParseURL = new URL[1];
        aParseURL[0] = new URL();
        aParseURL[0].Complete = unoURL;
        m_xParser.parseStrict(aParseURL);

        xDispatcher = xDisProv.queryDispatch(aParseURL[0], "", 0);
        assertNotNull("Can not obtain dispatcher for query: " + unoURL, xDispatcher);
        xDispatcher.dispatch(aParseURL[0], null);
    }

    /** @short checks the numbers displayed in the title

        @descr cycles through default view and print preview
               and asserts that the title doesn't change.
    */
    @Test public void checkTitleNumbers() throws Exception
    {
        PropertyValue[] lArgs = new PropertyValue[1];

        lArgs[0]         = new PropertyValue();
        lArgs[0].Name    = "Hidden";
        lArgs[0].Value   = Boolean.FALSE;

        // load doc
        XComponent xDoc=null;
        xDoc = m_xLoader.loadComponentFromURL("private:factory/swriter", "_blank", 0, lArgs);
        assertNotNull("Could not load temporary document", xDoc);

        XModel xModel = UnoRuntime.queryInterface( XModel.class, xDoc );
        XController xController = UnoRuntime.queryInterface( XController.class, xModel.getCurrentController() );
        XTitle xTitle = UnoRuntime.queryInterface( XTitle.class, xModel.getCurrentController().getFrame() );
        XDispatchProvider xDisProv = null;

        // get window title with ui in default mode
        String defaultTitle = xTitle.getTitle();

        xDisProv = UnoRuntime.queryInterface( XDispatchProvider.class, xModel.getCurrentController() );
        prepareQueryAndDispatch( xDisProv, UNO_URL_FOR_PRINT_PREVIEW );
        utils.waitForEventIdle(m_xMSF);
        // get window title with ui in print preview mode
        String printPreviewTitle = xTitle.getTitle();
        assertEquals("Title mismatch between default view window title and print preview window title",
                     defaultTitle, printPreviewTitle);

        xDisProv = UnoRuntime.queryInterface( XDispatchProvider.class, xModel.getCurrentController() );
        prepareQueryAndDispatch( xDisProv, UNO_URL_FOR_CLOSING_PRINT_PREVIEW );
        utils.waitForEventIdle(m_xMSF);
        //get window title with ui back in default mode
        String printPreviewClosedTitle = xTitle.getTitle();
        assertEquals("Title mismatch between default view window title and title after switching from print preview to default view window"                     ,defaultTitle, printPreviewClosedTitle);

        xDisProv = UnoRuntime.queryInterface( XDispatchProvider.class, xModel.getCurrentController() );
        prepareQueryAndDispatch( xDisProv, UNO_URL_FOR_CLOSING_DOC );

        xDoc     = null;
        xDisProv = null;
    }

    /** checks the if SuggestedSaveAsName is displayed in the title */
    @Test
    public void checkTitleSuggestedFileName() throws Exception
    {
        PropertyValue[] lArgs = new PropertyValue[2];

        lArgs[0]         = new PropertyValue();
        lArgs[0].Name    = "Hidden";
        lArgs[0].Value   = Boolean.FALSE;
        lArgs[1]         = new PropertyValue();
        lArgs[1].Name    = "SuggestedSaveAsName";
        lArgs[1].Value   = "suggestme.odt";

        // load doc
        XComponent xDoc = m_xLoader.loadComponentFromURL("private:factory/swriter", "_blank", 0, lArgs);
        assertNotNull("Could not load temporary document", xDoc);

        XModel xModel = UnoRuntime.queryInterface( XModel.class, xDoc );
        XTitle xTitle = UnoRuntime.queryInterface( XTitle.class, xModel.getCurrentController().getFrame() );

        String title = xTitle.getTitle();
        assertTrue(title.startsWith("suggestme.odt"));

        XDispatchProvider xDisProv = UnoRuntime.queryInterface( XDispatchProvider.class, xModel.getCurrentController() );
        prepareQueryAndDispatch( xDisProv, UNO_URL_FOR_CLOSING_DOC );
    }

    /** @short sets frame title and checks for infinite recursion

        @descr sets frame title. then cycles through default and
               print preview. then closes the window and checks
               for infinite recursion.
    */
    @Test public void setTitleAndCheck() throws Exception
    {
        PropertyValue[] lArgs = new PropertyValue[1];

        lArgs[0]       = new PropertyValue();
        lArgs[0].Name  = "Hidden";
        lArgs[0].Value = Boolean.FALSE;

        // load doc
        XComponent xDoc = null;
        xDoc = m_xLoader.loadComponentFromURL("private:factory/swriter", "_blank", 0, lArgs);
        assertNotNull("Could not create office document", xDoc);
        XModel xModel  = UnoRuntime.queryInterface( XModel.class, xDoc );
        XFrame2 xFrame = UnoRuntime.queryInterface( XFrame2.class, xModel.getCurrentController().getFrame() );
        XDispatchProvider xDisProv = null;
        // set doc title
        xFrame.setTitle(DOCUMENT_TITLE);

        // switch to print preview mode
        xDisProv = UnoRuntime.queryInterface( XDispatchProvider.class, xModel.getCurrentController() );
        prepareQueryAndDispatch( xDisProv, UNO_URL_FOR_PRINT_PREVIEW );
        utils.waitForEventIdle(m_xMSF);

        // switch back to default mode
        xDisProv = UnoRuntime.queryInterface( XDispatchProvider.class, xModel.getCurrentController() );
        prepareQueryAndDispatch( xDisProv, UNO_URL_FOR_CLOSING_PRINT_PREVIEW );
        utils.waitForEventIdle(m_xMSF);

        Thread.sleep(250); // FIXME why is this needed?

        // close document
        xDisProv = UnoRuntime.queryInterface( XDispatchProvider.class, xModel.getCurrentController() );
        try{
                prepareQueryAndDispatch( xDisProv, UNO_URL_FOR_CLOSING_DOC );
        } catch( Exception e ) {
            fail(e.toString());
        }

        xDoc     = null;
        xDisProv = null;
    }

    /** @short checks creation of new empty document with readonly set to true

        @descr creation of a new document with readonly property set
               to true.  this should not fail even if as such it can
               be seen as a slightly silly thing to do. But existing
               3rd-party code depends on it to work.
    */
    @Test public void checkLoadingWithReadOnly()
    {
        PropertyValue[] lArgs = new PropertyValue[2];

        lArgs[0]         = new PropertyValue();
        lArgs[0].Name    = "Hidden";
        lArgs[0].Value   = Boolean.FALSE;
        lArgs[1]         = new PropertyValue();
        lArgs[1].Name    = "ReadOnly";
        lArgs[1].Value   = Boolean.TRUE;

        // load doc
        XComponent xDoc = null;
        try{
                xDoc = m_xLoader.loadComponentFromURL("private:factory/swriter", "_default", 0, lArgs);
        } catch (Exception e) {
        }
        assertNotNull("Creating a new document read with ReadOnly property true should work (even if slightly silly)", xDoc);
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
