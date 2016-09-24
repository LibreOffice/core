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
package complex.sfx2.tools;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XURLTransformer;

import util.AccessibilityTools;
import util.WriterTools;

/**
 * Methods to open Writer docs
 *
 */
public class WriterHelper {
    private XMultiServiceFactory m_xMSF = null;

    /**
     * Creates a new instance of WriterHelper
     *
     * @param xMSF
     *            The MultiServiceFactory gained from the office
     */
    public WriterHelper(XMultiServiceFactory xMSF) {
        this.m_xMSF = xMSF;
    }

    /**
     * Opens an empty document
     *
     * @return a reference to the opened document is returned
     */
    public XTextDocument openEmptyDoc() {
        return WriterTools.createTextDoc(m_xMSF);
    }

    /**
     * Closes a given XTextDocument
     *
     * @param xTextDoc
     *            the text document to be closed
     * @return if an error occurs the errormessage is returned and an empty
     *         String if not
     */
    public String closeDoc(XTextDocument xTextDoc) {
        XCloseable closer = UnoRuntime.queryInterface(XCloseable.class,
                xTextDoc);
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

    /**
     * a TextDocument is opened by pressing a button in a dialog given by
     * uno-URL
     *
     * @param url
     *            the uno-URL of the dialog to be opened
     * @param createButton
     *            the language dependent label of the button to be pressed
     * @param destroyLocal
     *            if true the document that has been opened to dispatch the
     *            dialog is closed before the method returns, otherwise this
     *            document remains open
     * @return returns the created Textdocument
     */
    public XTextDocument openFromDialog(String url, String createButton,
            boolean destroyLocal) throws Exception {
        xLocalDoc = WriterTools.createTextDoc(m_xMSF);
        XComponent comp = UnoRuntime
                .queryInterface(XComponent.class, xLocalDoc);

        XModel aModel = UnoRuntime.queryInterface(XModel.class, comp);

        XController xController = aModel.getCurrentController();

        // Opening Dialog
        XDispatchProvider xDispProv = UnoRuntime.queryInterface(
                XDispatchProvider.class, xController.getFrame());
        XURLTransformer xParser = UnoRuntime.queryInterface(
                XURLTransformer.class,
                m_xMSF.createInstance("com.sun.star.util.URLTransformer"));

        // Because it's an in/out parameter
        // we must use an array of URL objects.
        URL[] aParseURL = new URL[] { new URL() };
        aParseURL[0].Complete = url;
        xParser.parseStrict(aParseURL);

        XDispatch xDispatcher = xDispProv.queryDispatch(aParseURL[0], "",
                com.sun.star.frame.FrameSearchFlag.SELF
                        | com.sun.star.frame.FrameSearchFlag.CHILDREN);
        if (xDispatcher != null) {
            PropertyValue[] dispatchArguments = new PropertyValue[0];
            xDispatcher.dispatch(aParseURL[0], dispatchArguments);
        }

        if (createButton.length() > 1) {
            XExtendedToolkit tk = getToolkit();
            Object atw = tk.getActiveTopWindow();

            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, atw);

            XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);
            XAccessibleContext buttonContext = AccessibilityTools
                    .getAccessibleObjectForRole(xRoot,
                            AccessibleRole.PUSH_BUTTON, createButton);

            XAccessibleAction buttonAction = UnoRuntime.queryInterface(
                    XAccessibleAction.class, buttonContext);

            try {
                System.out
                        .println("Name: " + buttonContext.getAccessibleName());
                buttonAction.doAccessibleAction(0);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                System.out.println("Couldn't press button");
            }

            util.utils.waitForEventIdle(m_xMSF);
        }

        XDesktop xDesktop = getDesktop();

        XTextDocument returnDoc = UnoRuntime.queryInterface(
                XTextDocument.class, xDesktop.getCurrentComponent());

        if (destroyLocal) {
            closeDoc(xLocalDoc);
            xLocalDoc = null;
        }

        return returnDoc;
    }

    public void closeFromDialog() {
        closeDoc(xLocalDoc);
        xLocalDoc = null;
    }

    /**
     * creates an instance of com.sun.star.awt.Toolkit to query the
     * XExtendedToolkit interface
     *
     * @return returns the gained XExtendedToolkit Interface
     */
    public XExtendedToolkit getToolkit() throws com.sun.star.uno.Exception {
        Object toolkit = m_xMSF.createInstance("com.sun.star.awt.Toolkit");

        XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class,
                toolkit);

        return tk;
    }

    /**
     * creates an instance of com.sun.star.frame.Desktop to query the XDesktop
     * interface
     *
     * @return returns the gained XDesktop interface
     */
    private XDesktop getDesktop() throws com.sun.star.uno.Exception {
        Object desk = m_xMSF.createInstance("com.sun.star.frame.Desktop");

        XDesktop xDesktop = UnoRuntime.queryInterface(XDesktop.class, desk);

        return xDesktop;
    }
}