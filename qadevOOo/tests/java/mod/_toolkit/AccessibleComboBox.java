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
package mod._toolkit;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;


/**
 * Test for object which is represented by accessible component
 * of the 'Font' combo box in Format->Character... dislog box. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleExtendedComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleContext</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleContext
 */
public class AccessibleComboBox extends TestCase {
    private static XTextDocument xTextDoc = null;
    private static XAccessibleAction action = null;

    /**
     * Opens CharacterStyle dialog and
     * finds AccessibleComboBox walking through the
     * accessible component tree.
     */
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) {
        XInterface oObj = null;

        try {
            oObj = (XInterface) ((XMultiServiceFactory) Param.getMSF()).createInstance(
                           "com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, oObj);

        DiagThread psDiag = new DiagThread(xTextDoc,
                                           (XMultiServiceFactory) Param.getMSF());
        psDiag.start();

        util.utils.shortWait(Param.getInt("ShortWait"));

        AccessibilityTools at = new AccessibilityTools();

        util.utils.shortWait(Param.getInt("ShortWait"));

        Object atw = tk.getActiveTopWindow();

        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                              atw);

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PUSH_BUTTON,
                                             "Cancel");
        action = (XAccessibleAction) UnoRuntime.queryInterface(
                         XAccessibleAction.class, oObj);

        oObj = at.getAccessibleObjectForRole(xRoot,
                                             AccessibleRole.PAGE_TAB_LIST);

        XAccessibleSelection xAccSel = (XAccessibleSelection) UnoRuntime.queryInterface(
                                               XAccessibleSelection.class,
                                               oObj);

        try {
            xAccSel.selectAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        }
        util.utils.shortWait(Param.getInt("ShortWait"));

        at.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL, "",
                                             "AccessibleComboBox");

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XAccessibleComponent acomp = (XAccessibleComponent) UnoRuntime.queryInterface(
                                                   XAccessibleComponent.class,
                                                   oObj);
        final XAccessibleComponent acomp1 = (XAccessibleComponent) UnoRuntime.queryInterface(
                                                    XAccessibleComponent.class,
                                                    action);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                acomp1.grabFocus();
                acomp.grabFocus();
            }
        });

        return tEnv;
    }

    /**
     * Closes the dialog using accessible button 'Cancel' found in
     * <code>createTestEnvironment()</code>.
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("    closing dialog ");

        try {
            action.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Couldn't close dialog");
        } catch (com.sun.star.lang.DisposedException de) {
            log.println("Dialog already disposed");
        }

        util.DesktopTools.closeDoc(xTextDoc);
        ;
    }

    /**
     * Creates writer doc
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory(
                                         (XMultiServiceFactory) Param.getMSF());
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create document", e);
        }
    }


    /**
     * Thread for opening modal dialog 'Character style'.
     */
    public class DiagThread extends Thread {
        public XTextDocument xTextDoc = null;
        public XMultiServiceFactory msf = null;

        public DiagThread(XTextDocument xTextDoc, XMultiServiceFactory msf) {
            this.xTextDoc = xTextDoc;
            this.msf = msf;
        }

        public void run() {
            XModel aModel = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                               xTextDoc);

            XController xController = aModel.getCurrentController();

            //Opening PrinterSetupDialog
            try {
                String aSlotID = ".uno:FontDialog";
                XDispatchProvider xDispProv = (XDispatchProvider) UnoRuntime.queryInterface(
                                                      XDispatchProvider.class,
                                                      xController);
                XURLTransformer xParser = (com.sun.star.util.XURLTransformer) UnoRuntime.queryInterface(
                                                  XURLTransformer.class,
                                                  msf.createInstance(
                                                          "com.sun.star.util.URLTransformer"));

                // Because it's an in/out parameter we must use an array of URL objects.
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