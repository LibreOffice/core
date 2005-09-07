/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CheckGlobalEventBroadcaster_writer1.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:25:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package complex.framework;

import com.sun.star.awt.XWindow;
import com.sun.star.document.XEventBroadcaster;
import com.sun.star.document.XEventListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import complex.framework.DocHelper.WriterHelper;
import complexlib.ComplexTestCase;
import java.util.ArrayList;
import com.sun.star.task.XJobExecutor;
import com.sun.star.util.URL;
import util.UITools;

/**
 * This testcase checks the GlobalEventBroadcaster
 * it will add an XEventListener and verify the Events
 * raised when opening/changing and closing Office Documents
 */
public class CheckGlobalEventBroadcaster_writer1 extends ComplexTestCase {
    XMultiServiceFactory m_xMSF = null;
    XEventBroadcaster m_xEventBroadcaster = null;
    ArrayList notifyEvents = new ArrayList();
    XTextDocument xTextDoc;
    XSpreadsheetDocument xSheetDoc;
    XEventListener m_xEventListener = new EventListenerImpl();

    public String[] getTestMethodNames() {
        return new String[] {
            "initialize", "checkWriter", "cleanup"
        };
    }

    public void initialize() {
        m_xMSF = (XMultiServiceFactory) param.getMSF();
        log.println("check wether there is a valid MultiServiceFactory");

        if (m_xMSF == null) {
            assure("## Couldn't get MultiServiceFactory make sure your Office is started",
                   true);
        }

        log.println("... done");

        log.println(
                "Create an instance of com.sun.star.frame.GlobalEventBroadcaster");

        Object GlobalEventBroadcaster = null;
        Object dispatcher = null;

        try {
            GlobalEventBroadcaster = m_xMSF.createInstance(
                                             "com.sun.star.frame.GlobalEventBroadcaster");
        } catch (com.sun.star.uno.Exception e) {
            assure("## Exception while creating instance", false);
        }

        log.println("... done");

        log.println("check wether the created instance is valid");

        if (GlobalEventBroadcaster == null) {
            assure("couldn't create service", false);
        }

        log.println("... done");

        log.println(
                "try to query the XEventBroadcaster from the gained Object");
        m_xEventBroadcaster = (XEventBroadcaster) UnoRuntime.queryInterface(
                                      XEventBroadcaster.class,
                                      GlobalEventBroadcaster);

        if (util.utils.isVoid(m_xEventBroadcaster)) {
            assure("couldn't get XEventBroadcaster", false);
        }

        log.println("... done");

        log.println("adding Listener");
        m_xEventBroadcaster.addEventListener(m_xEventListener);
        log.println("... done");
    }

    public void checkWriter() {
        log.println("-- Checking Writer --");

        WriterHelper wHelper = new WriterHelper(m_xMSF);
        String[] expected;
        boolean locRes = true;
        log.println("opening an empty writer doc");
        notifyEvents.clear();
        xTextDoc = wHelper.openEmptyDoc();
        shortWait();
        expected = new String[] { "OnUnfocus", "OnCreate", "OnViewCreated", "OnFocus" };

        assure("Wrong events fired when opening empty doc",
               proveExpectation(expected));
        log.println("... done");

        log.println("changing the writer doc");
        notifyEvents.clear();
        xTextDoc.getText().setString("GlobalEventBroadcaster");
        shortWait();
        expected = new String[] { "OnModifyChanged" };

        assure("Wrong events fired when changing doc",
               proveExpectation(expected));
        log.println("... done");

        log.println("closing the empty writer doc");
        notifyEvents.clear();
        wHelper.closeDoc(xTextDoc);
        shortWait();
        expected = new String[] { "OnUnfocus", "OnFocus", "OnViewClosed", "OnUnload" };

        assure("Wrong events fired when closing empty doc",
               proveExpectation(expected));
        log.println("... done");

        log.println("opening an writer doc via Window-New Window");
        notifyEvents.clear();
        xTextDoc = wHelper.openFromDialog(".uno:NewWindow", "", false);
        shortWait();
        expected = new String[] { "OnUnfocus", "OnCreate", "OnViewCreated", "OnFocus", "OnUnfocus", "OnViewCreated", "OnFocus", };

        assure("Wrong events fired when opening an writer doc via Window-New Window",
               proveExpectation(expected));
        log.println("... done");

        log.println("closing the created writer doc");
        notifyEvents.clear();

        wHelper.closeDoc(xTextDoc);
        shortWait();
        expected = new String[] { "OnViewClosed", "OnUnfocus", "OnFocus", "OnViewClosed", "OnUnload" };

        assure("Wrong events fired when closing Window-New Window",
               proveExpectation(expected));

        log.println("... done");

    log.println("Opening document with label wizard");
    xTextDoc = wHelper.openFromDialog("private:factory/swriter?slot=21051", "", false);
        shortWait();
        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, wHelper.getToolkit ().getActiveTopWindow ());
        UITools ut = new UITools(m_xMSF,xWindow);
        notifyEvents.clear();
        log.println("pressing button 'New Document'");
        try{
            ut.clickButton ("New Document");
        } catch (Exception e) {
            log.println("Couldn't press Button");
        }
        log.println("... done");
        shortWait();
        shortWait();
    shortWait();
        expected = new String[] { "OnViewClosed", "OnCreate", "OnFocus", "OnModifyChanged" };

        assure("Wrong events fired when starting labels wizard",
               proveExpectation(expected));

        log.println("-- Done Writer --");
    }

    public void cleanup() {
        log.println("removing Listener");
        m_xEventBroadcaster.removeEventListener(m_xEventListener);
        log.println("... done");
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            log.println("While waiting :" + e);
        }
    }

    private boolean proveExpectation(String[] expected) {
        boolean locRes = true;
                boolean failure = false;

        log.println("Fired Events:");
        for (int k=0;k<notifyEvents.size();k++) {
            System.out.println("\t- "+notifyEvents.get(k));
        }

        for (int i = 0; i < expected.length; i++) {
            locRes = notifyEvents.contains(expected[i]);

            if (!locRes) {
                log.println("The event " + expected[i] + " isn't fired");
                                failure = true;
            }
        }

        return !failure;
    }

    public class EventListenerImpl implements XEventListener {
        public void disposing(com.sun.star.lang.EventObject eventObject) {
            log.println("disposing: " + eventObject.Source.toString());
        }

        public void notifyEvent(com.sun.star.document.EventObject eventObject) {
            notifyEvents.add(eventObject.EventName);
        }
    }
}
