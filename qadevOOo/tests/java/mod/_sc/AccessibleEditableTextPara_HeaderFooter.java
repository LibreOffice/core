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

        XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, toolkit);

        util.utils.shortWait(Param.getInt("ShortWait"));

        DiagThread psDiag = new DiagThread(xCalcDoc, msf);
        psDiag.start();

        util.utils.shortWait(Param.getInt("ShortWait")*5);

        Object atw = tk.getActiveTopWindow();

        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                              atw);

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        util.utils.shortWait(Param.getInt("ShortWait"));

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

        action = (XAccessibleAction) UnoRuntime.queryInterface(
                         XAccessibleAction.class, ok_button);

        XAccessibleText text = (XAccessibleText) UnoRuntime.queryInterface(
                                       XAccessibleText.class, oObj);

        XAccessibleEditableText eText = (XAccessibleEditableText) UnoRuntime.queryInterface(
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
    protected void initialize(TestParameters Param, PrintWriter log) {
        msf = (XMultiServiceFactory) Param.getMSF();

        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory(msf);
            xCalcDoc = (XComponent) UnoRuntime.queryInterface(XComponent.class,
                                                              SOF.createCalcDoc(
                                                                      null));
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create document", e);
        }
    }


    /**
     * Thread for opening modal dialog 'Print Settings'.
     */
    public class DiagThread extends Thread {
        public XComponent xCalcDoc = null;
        public XMultiServiceFactory msf = null;

        public DiagThread(XComponent xCalcDoc, XMultiServiceFactory msf) {
            this.xCalcDoc = xCalcDoc;
            this.msf = msf;
        }

        public void run() {
            XModel aModel = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                               xCalcDoc);

            XController xController = aModel.getCurrentController();

            //Opening HeaderFooterDialog
            try {
                String aSlotID = ".uno:EditHeaderAndFooter";
                XDispatchProvider xDispProv = (XDispatchProvider) UnoRuntime.queryInterface(
                                                      XDispatchProvider.class,
                                                      xController);
                XURLTransformer xParser = (com.sun.star.util.XURLTransformer) UnoRuntime.queryInterface(
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