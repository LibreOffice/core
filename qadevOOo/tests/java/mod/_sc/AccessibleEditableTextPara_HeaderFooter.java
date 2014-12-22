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
package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;


public class AccessibleEditableTextPara_HeaderFooter extends TestCase {
    protected static XComponent xCalcDoc = null;
    protected static XMultiServiceFactory msf = null;
    protected static XAccessibleAction action = null;

    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) {
        XAccessibleContext oObj = null;
        Object toolkit = null;

        try {
            toolkit = msf.createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        XExtendedToolkit tk = UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, toolkit);

        util.utils.pause(Param.getInt("ShortWait"));

        DiagThread psDiag = new DiagThread(xCalcDoc, msf);
        psDiag.start();

        util.utils.pause(Param.getInt("ShortWait")*5);

        Object atw = tk.getActiveTopWindow();

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,
                                                              atw);

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        util.utils.pause(Param.getInt("ShortWait"));

        AccessibilityTools.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        XAccessibleContext ok_button = AccessibilityTools.getAccessibleObjectForRole(xRoot,
                                                                     AccessibleRole.PUSH_BUTTON,
                                                                     "Cancel");

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PARAGRAPH);

        log.println("ImplementationName " + utils.getImplName(oObj));
        log.println("AccessibleName " + oObj.getAccessibleName());
        log.println("ParentDescription " +
                    oObj.getAccessibleParent().getAccessibleContext()
                        .getAccessibleDescription());

        TestEnvironment tEnv = new TestEnvironment(oObj);

        action = UnoRuntime.queryInterface(
                         XAccessibleAction.class, ok_button);

        XAccessibleText text = UnoRuntime.queryInterface(
                                       XAccessibleText.class, oObj);

        XAccessibleEditableText eText = UnoRuntime.queryInterface(
                                                XAccessibleEditableText.class,
                                                oObj);

        eText.setText("LEFT");

        tEnv.addObjRelation("XAccessibleText.Text", text.getText());

        tEnv.addObjRelation("Destroy", "AccessibleEditableTextPara_PreviewCell");

        final XAccessibleEditableText editText = eText;

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                editText.setText("LEFT_EVENT");
            }
        });

        return tEnv;
    }

    /**
     * Closes the dialog using accessible button 'OK' found in
     * <code>createTestEnvironment()</code>.
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("    disposing xCalcDoc ");

        try {
            action.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Couldn't close dialog");
        } catch (com.sun.star.lang.DisposedException de) {
            log.println("Dialog already disposed");
        }

        util.DesktopTools.closeDoc(xCalcDoc);
    }

    /**
     * Opens new writer document.
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) {
        msf = Param.getMSF();

        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory(msf);
            xCalcDoc = UnoRuntime.queryInterface(XComponent.class,
                                                              SOF.createCalcDoc(
                                                                      null));
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create document", e);
        }
    }


    /**
     * Thread for opening modal dialog 'Print Settings'.
     */
    private class DiagThread extends Thread {
        private XComponent xCalcDoc = null;
        private XMultiServiceFactory msf = null;

        private DiagThread(XComponent xCalcDoc, XMultiServiceFactory msf) {
            this.xCalcDoc = xCalcDoc;
            this.msf = msf;
        }

        @Override
        public void run() {
            XModel aModel = UnoRuntime.queryInterface(XModel.class,
                                                               xCalcDoc);

            XController xController = aModel.getCurrentController();

            //Opening HeaderFooterDialog
            try {
                String aSlotID = ".uno:EditHeaderAndFooter";
                XDispatchProvider xDispProv = UnoRuntime.queryInterface(
                                                      XDispatchProvider.class,
                                                      xController);
                XURLTransformer xParser = UnoRuntime.queryInterface(
                                                  XURLTransformer.class,
                                                  msf.createInstance(
                                                          "com.sun.star.util.URLTransformer"));

                // Because it's an in/out parameter
                // we must use an array of URL objects.
                URL[] aParseURL = new URL[1];
                aParseURL[0] = new URL();
                aParseURL[0].Complete = aSlotID;
                xParser.parseStrict(aParseURL);

                URL aURL = aParseURL[0];
                XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);

                if (xDispatcher != null) {
                    xDispatcher.dispatch(aURL, null);
                }
            } catch (com.sun.star.uno.Exception e) {
                log.println("Couldn't open dialog");
            }
        }
    }
}