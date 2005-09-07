/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WriterHelper.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:26:09 $
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
package complex.framework.DocHelper;

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

import complex.framework.DocHelper.DialogThread;
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
        XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                                    XCloseable.class, xTextDoc);
        String err = "";

        try {
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            err = "couldn't close document " + e;
        }

        return err;
    }

    /** a TextDocument is opened by pressing a button in a dialog given by uno-URL
     * @param url the uno-URL of the dialog to be opened
     * @param createButton the language dependend label of the button to be pressed
     * @param destroyLocal if true the document that has been opened to dispatch the dialog is closed before the method returns,
     * otherwise this document remains open
     * @return returns the created Textdocument
     */
    public XTextDocument openFromDialog(String url, String createButton,
                                        boolean destroyLocal) {
        XTextDocument xLocalDoc = WriterTools.createTextDoc(m_xMSF);
        XComponent comp = (XComponent) UnoRuntime.queryInterface(
                                  XComponent.class, xLocalDoc);
        DialogThread diagThread = new DialogThread(comp, m_xMSF, url);
        diagThread.start();
        shortWait();

        if (createButton.length() > 1) {
            XExtendedToolkit tk = getToolkit();
            AccessibilityTools at = new AccessibilityTools();
            Object atw = tk.getActiveTopWindow();

            XWindow xWindow = (XWindow) UnoRuntime.queryInterface(
                                      XWindow.class, atw);

            XAccessible xRoot = at.getAccessibleObject(xWindow);
            XAccessibleContext buttonContext = at.getAccessibleObjectForRole(
                                                       xRoot,
                                                       AccessibleRole.PUSH_BUTTON,
                                                       createButton);

            XAccessibleAction buttonAction = (XAccessibleAction) UnoRuntime.queryInterface(
                                                     XAccessibleAction.class,
                                                     buttonContext);

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

        XTextDocument returnDoc = (XTextDocument) UnoRuntime.queryInterface(
                                          XTextDocument.class,
                                          xDesktop.getCurrentComponent());

        if (destroyLocal) {
            closeDoc(xLocalDoc);
        }

        return returnDoc;
    }

    public XTextDocument DocByAutopilot(XMultiServiceFactory msf,
                                        int[] indexes, boolean destroyLocal,
                                        String bName) {
        XTextDocument xLocalDoc = WriterTools.createTextDoc(m_xMSF);
        Object toolkit = null;

        try {
            toolkit = msf.createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, toolkit);

        shortWait();

        AccessibilityTools at = new AccessibilityTools();

        Object atw = tk.getActiveTopWindow();

        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                              atw);

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        XAccessibleContext ARoot = at.getAccessibleObjectForRole(xRoot,
                                                                 AccessibleRole.MENU_BAR);
        XAccessibleSelection sel = (XAccessibleSelection) UnoRuntime.queryInterface(
                                           XAccessibleSelection.class, ARoot);

        for (int k = 0; k < indexes.length; k++) {
            try {
                sel.selectAccessibleChild(indexes[k]);
                shortWait();
                ARoot = ARoot.getAccessibleChild(indexes[k])
                             .getAccessibleContext();
                sel = (XAccessibleSelection) UnoRuntime.queryInterface(
                              XAccessibleSelection.class, ARoot);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            }
        }

        shortWait();

        atw = tk.getActiveTopWindow();

        xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, atw);

        xRoot = at.getAccessibleObject(xWindow);

        //at.printAccessibleTree(new PrintWriter(System.out),xRoot);

        XAccessibleAction action = (XAccessibleAction) UnoRuntime.queryInterface(
                                           XAccessibleAction.class,
                                           at.getAccessibleObjectForRole(xRoot,
                                                                         AccessibleRole.PUSH_BUTTON,
                                                                         bName));

        try {
            action.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        shortWait();

        atw = tk.getActiveTopWindow();

        xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, atw);

        xRoot = at.getAccessibleObject(xWindow);

        at.printAccessibleTree(new PrintWriter(System.out),xRoot);

        action = (XAccessibleAction) UnoRuntime.queryInterface(
                                           XAccessibleAction.class,
                                           at.getAccessibleObjectForRole(xRoot,
                                                                         AccessibleRole.PUSH_BUTTON,
                                                                         "Yes"));

        try {
            if (action != null) action.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        shortWait();

        XDesktop xDesktop = getDesktop();

        XTextDocument returnDoc = (XTextDocument) UnoRuntime.queryInterface(
                                          XTextDocument.class,
                                          xDesktop.getCurrentComponent());

        if (destroyLocal) {
            closeDoc(xLocalDoc);
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
            e.printStackTrace();
        }

        XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, toolkit);

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
            e.printStackTrace();
        }

        XDesktop xDesktop = (XDesktop) UnoRuntime.queryInterface(
                                    XDesktop.class, desk);

        return xDesktop;
    }
}