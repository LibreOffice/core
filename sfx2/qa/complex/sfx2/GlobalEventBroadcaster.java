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
package complex.sfx2;

import com.sun.star.awt.XWindow;
import com.sun.star.document.XEventBroadcaster;
import com.sun.star.document.XEventListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import complex.sfx2.tools.WriterHelper;

import java.util.ArrayList;

import util.UITools;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;


/**
 * This testcase checks the GlobalEventBroadcaster
 * it will add an XEventListener and verify the Events
 * raised when opening/changing and closing Office Documents
 */
public class GlobalEventBroadcaster {
    XMultiServiceFactory m_xMSF = null;
    XEventBroadcaster m_xEventBroadcaster = null;
    ArrayList<String> notifyEvents = new ArrayList<String>();
    // XTextDocument xTextDoc;
    XSpreadsheetDocument xSheetDoc;
    XEventListener m_xEventListener = new EventListenerImpl();

    @Before public void initialize() {
        m_xMSF = getMSF();
        System.out.println("check whether there is a valid MultiServiceFactory");

        assertNotNull("## Couldn't get MultiServiceFactory make sure your Office is started", m_xMSF);

        System.out.println("... done");

        System.out.println(
                "Create an instance of com.sun.star.frame.GlobalEventBroadcaster");

        Object GlobalEventBroadcaster = null;

        try {
            GlobalEventBroadcaster = m_xMSF.createInstance(
                                             "com.sun.star.frame.GlobalEventBroadcaster");
        } catch (com.sun.star.uno.Exception e) {
            fail("## Exception while creating instance");
        }

        System.out.println("... done");

        System.out.println("check whether the created instance is valid");

        assertNotNull("couldn't create service", GlobalEventBroadcaster);

        System.out.println("... done");

        System.out.println(
                "try to query the XEventBroadcaster from the gained Object");
        m_xEventBroadcaster = UnoRuntime.queryInterface(XEventBroadcaster.class, GlobalEventBroadcaster);

        if (util.utils.isVoid(m_xEventBroadcaster)) {
            fail("couldn't get XEventBroadcaster");
        }

        System.out.println("... done");

        System.out.println("adding Listener");
        m_xEventBroadcaster.addEventListener(m_xEventListener);
        System.out.println("... done");
    }

    @Test public void checkWriter() {
        System.out.println("-- Checking Writer --");

        WriterHelper wHelper = new WriterHelper(m_xMSF);
        String[] expected;
        System.out.println("opening an empty writer doc");
        notifyEvents.clear();
        {
            XTextDocument xTextDoc = wHelper.openEmptyDoc();
            util.utils.pause(2000);
            expected = new String[] { "OnUnfocus", "OnCreate", "OnViewCreated", "OnFocus" };

            assertTrue("Wrong events fired when opening empty doc",
                   proveExpectation(expected));
            System.out.println("... done");

            System.out.println("changing the writer doc");
            notifyEvents.clear();
            xTextDoc.getText().setString("GlobalEventBroadcaster");
            util.utils.pause(2000);
            expected = new String[] { "OnModifyChanged" };

            assertTrue("Wrong events fired when changing doc",
                   proveExpectation(expected));
            System.out.println("... done");

            System.out.println("closing the empty writer doc");
            notifyEvents.clear();
            wHelper.closeDoc(xTextDoc);
            util.utils.pause(2000);
        }
        expected = new String[] { "OnUnfocus", "OnFocus", "OnViewClosed", "OnUnload" };

        assertTrue("Wrong events fired when closing empty doc",
               proveExpectation(expected));
        System.out.println("... done");

        System.out.println("opening an writer doc via Window-New Window");
        notifyEvents.clear();
        {
            XTextDocument xTextDoc = wHelper.openFromDialog(".uno:NewWindow", "", false);

            util.utils.pause(2000);
            expected = new String[] { "OnUnfocus", "OnCreate", "OnViewCreated", "OnFocus", "OnUnfocus", "OnViewCreated", "OnFocus", };

            assertTrue("Wrong events fired when opening an writer doc via Window-New Window",
                   proveExpectation(expected));
            System.out.println("... done");

            System.out.println("closing the created writer doc");
            notifyEvents.clear();

            wHelper.closeDoc(xTextDoc);
            util.utils.pause(2000);
        }
        expected = new String[] { "OnViewClosed", "OnUnfocus", "OnFocus", "OnViewClosed", "OnUnload" };

        assertTrue("Wrong events fired when closing Window-New Window",
               proveExpectation(expected));

        System.out.println("... done");
        // TODO: It seems not possible to close the document without interactiv question
        //       there the follow test will not be execute
        if (false) {
            System.out.println("Opening document with label wizard");
            XTextDocument xTextDoc = wHelper.openFromDialog("private:factory/swriter?slot=21051", "", false);
            util.utils.pause(2000);
            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, wHelper.getToolkit().getActiveTopWindow());
            UITools ut = new UITools(xWindow);
            notifyEvents.clear();
            System.out.println("pressing button 'New Document'");
            try{
                ut.clickButton ("New Document");
            } catch (Exception e) {
                System.out.println("Couldn't press Button");
            }
            System.out.println("... done");
            util.utils.pause(2000);
            util.utils.pause(2000);
            util.utils.pause(2000);
            expected = new String[] { "OnViewClosed", "OnCreate", "OnFocus", "OnModifyChanged" };

            assertTrue("Wrong events fired when starting labels wizard",
                   proveExpectation(expected));

            System.out.println("Try to close document...");
            wHelper.closeDoc(xTextDoc);
            util.utils.pause(2000);
            wHelper.closeFromDialog();
            util.utils.pause(2000);
            xTextDoc = null;
        }

        System.out.println("-- Done Writer --");
    }

    @After public void cleanup() {
        System.out.println("removing Listener");
        m_xEventBroadcaster.removeEventListener(m_xEventListener);
        System.out.println("... done");
    }

    private boolean proveExpectation(String[] expected) {
        boolean locRes = true;
                boolean failure = false;

        System.out.println("Fired Events:");
        for (int k=0;k<notifyEvents.size();k++) {
            System.out.println("\t- "+notifyEvents.get(k));
        }

        for (int i = 0; i < expected.length; i++) {
            locRes = notifyEvents.contains(expected[i]);

            if (!locRes) {
                System.out.println("The event " + expected[i] + " isn't fired");
                                failure = true;
            }
        }

        return !failure;
    }

    private class EventListenerImpl implements XEventListener {
        public void disposing(com.sun.star.lang.EventObject eventObject) {
            System.out.println("disposing: " + eventObject.Source.toString());
        }

        public void notifyEvent(com.sun.star.document.EventObject eventObject) {
            notifyEvents.add(eventObject.EventName);
        }
    }

    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection() CheckGlobalEventBroadcaster_writer1");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
