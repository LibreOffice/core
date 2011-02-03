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
package complex.sfx2.tools;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XDesktop;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;

import java.io.PrintWriter;

import util.AccessibilityTools;
import util.WriterTools;


/**
 * Methods to open Writer docs
 *
 */
public class WriterHelper {
    XMultiServiceFactory m_xMSF = null;

    /** Creates a new instance of WriterHelper
     * @param m_xMSF The MultiServiceFactory gained from the office
     */
    public WriterHelper(XMultiServiceFactory m_xMSF) {
        this.m_xMSF = m_xMSF;
    }

    /** Opens an empty document
     * @return a reference to the opened document is returned
     */
    public XTextDocument openEmptyDoc() {
        return WriterTools.createTextDoc(m_xMSF);
    }

    /** Closes a given XTextDocument
     * @param xTextDoc the text document to be closed
     * @return if an error occurs the errormessage is returned and an empty String if not
     */
    public String closeDoc(XTextDocument xTextDoc) {
        XCloseable closer = UnoRuntime.queryInterface(XCloseable.class, xTextDoc);
        String err = "";

        try {
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            err = "couldn't close document " + e;
            System.out.println(err);
        }

        return err;
    }

    private XTextDocument xLocalDoc = null;
    /** a TextDocument is opened by pressing a button in a dialog given by uno-URL
     * @param url the uno-URL of the dialog to be opened
     * @param createButton the language dependend label of the button to be pressed
     * @param destroyLocal if true the document that has been opened to dispatch the dialog is closed before the method returns,
     * otherwise this document remains open
     * @return returns the created Textdocument
     */
    public XTextDocument openFromDialog(String url, String createButton,
                                        boolean destroyLocal) {
        xLocalDoc = WriterTools.createTextDoc(m_xMSF);
        XComponent comp = UnoRuntime.queryInterface(XComponent.class, xLocalDoc);
        DialogThread diagThread = new DialogThread(comp, m_xMSF, url);
        diagThread.start();
        shortWait();

        if (createButton.length() > 1) {
            XExtendedToolkit tk = getToolkit();
            Object atw = tk.getActiveTopWindow();

            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, atw);

            XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);
            XAccessibleContext buttonContext = AccessibilityTools.getAccessibleObjectForRole(
                                                       xRoot,
                                                       AccessibleRole.PUSH_BUTTON,
                                                       createButton);

            XAccessibleAction buttonAction = UnoRuntime.queryInterface(XAccessibleAction.class, buttonContext);

            try {
                System.out.println("Name: " +
                                   buttonContext.getAccessibleName());
                buttonAction.doAccessibleAction(0);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                System.out.println("Couldn't press button");
            }

            shortWait();
        }

        XDesktop xDesktop = getDesktop();

        XTextDocument returnDoc = UnoRuntime.queryInterface(XTextDocument.class, xDesktop.getCurrentComponent());

        if (destroyLocal) {
            closeDoc(xLocalDoc);
            xLocalDoc = null;
        }

        return returnDoc;
    }
    public void closeFromDialog()
    {
        closeDoc(xLocalDoc);
        xLocalDoc = null;
    }
    public void kill()
    {
        XDesktop xDesktop = getDesktop();
        xDesktop.terminate();
    }


    public XTextDocument DocByAutopilot(XMultiServiceFactory msf,
                                        int[] indexes, boolean destroyLocal,
                                        String bName) {
        XTextDocument xTextDoc = WriterTools.createTextDoc(m_xMSF);
        Object toolkit = null;

        try {
            toolkit = msf.createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( System.err );
        }

        XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class, toolkit);

        shortWait();

        Object atw = tk.getActiveTopWindow();

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, atw);

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        XAccessibleContext ARoot = AccessibilityTools.getAccessibleObjectForRole(xRoot,
                                                                 AccessibleRole.MENU_BAR);
        XAccessibleSelection sel = UnoRuntime.queryInterface(XAccessibleSelection.class, ARoot);

        for (int k = 0; k < indexes.length; k++) {
            try {
                sel.selectAccessibleChild(indexes[k]);
                shortWait();
                ARoot = ARoot.getAccessibleChild(indexes[k])
                             .getAccessibleContext();
                sel = UnoRuntime.queryInterface(XAccessibleSelection.class, ARoot);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            }
        }

        shortWait();

        atw = tk.getActiveTopWindow();

        xWindow = UnoRuntime.queryInterface(XWindow.class, atw);

        xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        //at.printAccessibleTree(new PrintWriter(System.out),xRoot);

        XAccessibleAction action = UnoRuntime.queryInterface(XAccessibleAction.class, AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PUSH_BUTTON, bName));

        try {
            action.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        shortWait();

        atw = tk.getActiveTopWindow();

        xWindow = UnoRuntime.queryInterface(XWindow.class, atw);

        xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.printAccessibleTree(new PrintWriter(System.out),xRoot);

        action = UnoRuntime.queryInterface(XAccessibleAction.class, AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PUSH_BUTTON, "Yes"));

        try {
            if (action != null) action.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        shortWait();

        XDesktop xDesktop = getDesktop();

        XTextDocument returnDoc = UnoRuntime.queryInterface(XTextDocument.class, xDesktop.getCurrentComponent());

        if (destroyLocal) {
            closeDoc(xTextDoc);
        }

        return returnDoc;
    }

    /**
    * Sleeps for 2 sec. to allow StarOffice to react
    */
    private void shortWait() {
        try {
            Thread.sleep(4000);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }

    /** creates an instance of com.sun.star.awt.Toolkit to query the XExtendedToolkit
     * interface
     * @return returns the gained XExtendedToolkit Interface
     */
    public XExtendedToolkit getToolkit() {
        Object toolkit = null;

        try {
            toolkit = m_xMSF.createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            System.out.println("Couldn't get toolkit");
            e.printStackTrace( System.err );
        }

        XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class, toolkit);

        return tk;
    }

    /** creates an instance of com.sun.star.frame.Desktop to query the XDesktop interface
     * @return returns the gained XDesktop interface
     */
    protected XDesktop getDesktop() {
        Object desk = null;

        try {
            desk = m_xMSF.createInstance("com.sun.star.frame.Desktop");
        } catch (com.sun.star.uno.Exception e) {
            System.out.println("Couldn't get desktop");
            e.printStackTrace( System.err );
        }

        XDesktop xDesktop = UnoRuntime.queryInterface(XDesktop.class, desk);

        return xDesktop;
    }
}