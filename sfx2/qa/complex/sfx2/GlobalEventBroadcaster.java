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
    ArrayList notifyEvents = new ArrayList();
    // XTextDocument xTextDoc;
    XSpreadsheetDocument xSheetDoc;
    XEventListener m_xEventListener = new EventListenerImpl();

    @Before public void initialize() {
        m_xMSF = getMSF();
        System.out.println("check wether there is a valid MultiServiceFactory");

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

        System.out.println("check wether the created instance is valid");

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
            shortWait();
            expected = new String[] { "OnUnfocus", "OnCreate", "OnViewCreated", "OnFocus" };

            assertTrue("Wrong events fired when opening empty doc",
                   proveExpectation(expected));
            System.out.println("... done");

            System.out.println("changing the writer doc");
            notifyEvents.clear();
            xTextDoc.getText().setString("GlobalEventBroadcaster");
            shortWait();
            expected = new String[] { "OnModifyChanged" };

            assertTrue("Wrong events fired when changing doc",
                   proveExpectation(expected));
            System.out.println("... done");

            System.out.println("closing the empty writer doc");
            notifyEvents.clear();
            wHelper.closeDoc(xTextDoc);
            shortWait();
        }
        expected = new String[] { "OnUnfocus", "OnFocus", "OnViewClosed", "OnUnload" };

        assertTrue("Wrong events fired when closing empty doc",
               proveExpectation(expected));
        System.out.println("... done");

        System.out.println("opening an writer doc via Window-New Window");
        notifyEvents.clear();
        {
            XTextDocument xTextDoc = wHelper.openFromDialog(".uno:NewWindow", "", false);

            shortWait();
            expected = new String[] { "OnUnfocus", "OnCreate", "OnViewCreated", "OnFocus", "OnUnfocus", "OnViewCreated", "OnFocus", };

            assertTrue("Wrong events fired when opening an writer doc via Window-New Window",
                   proveExpectation(expected));
            System.out.println("... done");

            System.out.println("closing the created writer doc");
            notifyEvents.clear();

            wHelper.closeDoc(xTextDoc);
            shortWait();
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
            shortWait();
            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, wHelper.getToolkit().getActiveTopWindow());
            UITools ut = new UITools(m_xMSF,xWindow);
            notifyEvents.clear();
            System.out.println("pressing button 'New Document'");
            try{
                ut.clickButton ("New Document");
            } catch (Exception e) {
                System.out.println("Couldn't press Button");
            }
            System.out.println("... done");
            shortWait();
            shortWait();
            shortWait();
            expected = new String[] { "OnViewClosed", "OnCreate", "OnFocus", "OnModifyChanged" };

            assertTrue("Wrong events fired when starting labels wizard",
                   proveExpectation(expected));

            System.out.println("Try to close document...");
            wHelper.closeDoc(xTextDoc);
            shortWait();
            wHelper.closeFromDialog();
            shortWait();
            xTextDoc = null;
        }

        System.out.println("-- Done Writer --");
    }

    @After public void cleanup() {
        System.out.println("removing Listener");
        m_xEventBroadcaster.removeEventListener(m_xEventListener);
        System.out.println("... done");
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
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

    public class EventListenerImpl implements XEventListener {
        public void disposing(com.sun.star.lang.EventObject eventObject) {
            System.out.println("disposing: " + eventObject.Source.toString());
        }

        public void notifyEvent(com.sun.star.document.EventObject eventObject) {
            notifyEvents.add(eventObject.EventName);
        }
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
        System.out.println("tearDownConnection() CheckGlobalEventBroadcaster_writer1");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
