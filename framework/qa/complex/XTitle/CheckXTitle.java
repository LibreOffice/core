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

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.fail;
import util.utils;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.Desktop;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFrame2;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XTitle;
import com.sun.star.frame.XController;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URLTransformer;
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

    private URL parseURL(String unoURL)
    {
        URL[] aParseURL = new URL[] { new URL() };
        aParseURL[0].Complete = unoURL;
        m_xParser.parseStrict(aParseURL);
        return aParseURL[0];
    }

    private void waitUntilDispatcherAvailable(XModel xModel, String unoURL)
        throws InterruptedException
    {
        utils.waitForEventIdle(m_xMSF);

        // On Windows, some events such as focus changes are handled
        // asynchronously using PostMessage (Windows message queue)
        // so the previous UI transition may still not have completed yet
        // even though we called waitForEventIdle.
        //
        // Loop a few times until the desired dispatcher is available, which
        // is a better indication that the UI transition has completed.

        XDispatchProvider xDisProv;
        XDispatch xDispatcher = null;
        URL parsed_url = parseURL(unoURL);

        for (int ntries = 1; ntries < 5; ++ntries) {
            xDisProv = UnoRuntime.queryInterface(
                XDispatchProvider.class, xModel.getCurrentController() );
            xDispatcher = xDisProv.queryDispatch(parsed_url, "", 0);
            if (xDispatcher != null)
                break;
            Thread.sleep(250);
        }
        assertNotNull("Can not obtain dispatcher for query: " + unoURL, xDispatcher);
    }


    // prepare an uno URL query and dispatch it
    private void prepareQueryAndDispatch(XDispatchProvider xDisProv, String unoURL)
    {
        XDispatch xDispatcher = null;
        URL parsed_url = parseURL(unoURL);

        xDispatcher = xDisProv.queryDispatch(parsed_url, "", 0);
        assertNotNull("Can not obtain dispatcher for query: " + unoURL, xDispatcher);
        xDispatcher.dispatch(parsed_url, null);
    }

    /** @short checks the numbers displayed in the title

        @descr cycles through default view and print preview
               and asserts that the title doesn't change.

        disabled until the waitUntilDispatcherAvailable can be replaced
    */
    public void checkTitleNumbers() throws Exception
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
        waitUntilDispatcherAvailable( xModel, UNO_URL_FOR_CLOSING_PRINT_PREVIEW );

        // get window title with ui in print preview mode
        String printPreviewTitle = xTitle.getTitle();
        assertEquals("Title mismatch between default view window title and print preview window title",
                     defaultTitle, printPreviewTitle);

        xDisProv = UnoRuntime.queryInterface( XDispatchProvider.class, xModel.getCurrentController() );
        prepareQueryAndDispatch( xDisProv, UNO_URL_FOR_CLOSING_PRINT_PREVIEW );
        waitUntilDispatcherAvailable( xModel, UNO_URL_FOR_CLOSING_DOC );

        //get window title with ui back in default mode
        String printPreviewClosedTitle = xTitle.getTitle();
        assertEquals("Title mismatch between default view window title and title after switching from print preview to default view window"                     ,defaultTitle, printPreviewClosedTitle);

        xDisProv = UnoRuntime.queryInterface( XDispatchProvider.class, xModel.getCurrentController() );
        prepareQueryAndDispatch( xDisProv, UNO_URL_FOR_CLOSING_DOC );

        xDoc     = null;
        xDisProv = null;
    }

    /** @short sets frame title and checks for infinite recursion

        @descr sets frame title. then cycles through default and
               print preview. then closes the window and checks
               for infinite recursion.

        disabled until the waitUntilDispatcherAvailable can be replaced
    */
    public void setTitleAndCheck() throws Exception
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
        waitUntilDispatcherAvailable( xModel, UNO_URL_FOR_CLOSING_PRINT_PREVIEW );

        // switch back to default mode
        xDisProv = UnoRuntime.queryInterface( XDispatchProvider.class, xModel.getCurrentController() );
        prepareQueryAndDispatch( xDisProv, UNO_URL_FOR_CLOSING_PRINT_PREVIEW );
        waitUntilDispatcherAvailable( xModel, UNO_URL_FOR_CLOSING_DOC );

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
