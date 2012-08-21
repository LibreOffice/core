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
package mod._svtools;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.DesktopTools;
import util.SOfficeFactory;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.view.XSelectionSupplier;

public class AccessibleBrowseBox extends TestCase {

    static XDesktop the_Desk;
    static XTextDocument xTextDoc;

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        the_Desk = UnoRuntime.queryInterface(
            XDesktop.class, DesktopTools.createDesktop((XMultiServiceFactory) Param.getMSF()));
    }

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            xTextDoc.dispose();
        }
    }

    /**
     * Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects. Subclasses should implement this
     * method to provide the implementation and related objects. The method is
     * called from <code>getTestEnvironment()</code>.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        log.println("creating a test environment");

        if (xTextDoc != null) {
            xTextDoc.dispose();        // get a soffice factory object
        }
        SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a text document");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }

        shortWait();

        XModel aModel1 = UnoRuntime.queryInterface(XModel.class, xTextDoc);

        XController secondController = aModel1.getCurrentController();


        XDispatchProvider aProv = UnoRuntime.queryInterface(XDispatchProvider.class,
            secondController);

        XDispatch getting = null;

        log.println("opening DatasourceBrowser");
        URL the_url = new URL();
        the_url.Complete = ".component:DB/DataSourceBrowser";
        getting = aProv.queryDispatch(the_url, "_beamer", 12);
        PropertyValue[] noArgs = new PropertyValue[0];
        getting.dispatch(the_url, noArgs);

        shortWait();

        XFrame the_frame1 = the_Desk.getCurrentFrame();

        if (the_frame1 == null) {
            log.println("Current frame was not found !!!");
        }

        XFrame the_frame2 = the_frame1.findFrame("_beamer", 4);

        the_frame2.setName("DatasourceBrowser");

        XInterface oObj = null;

        final XSelectionSupplier xSelect = UnoRuntime.queryInterface(
            XSelectionSupplier.class, the_frame2.getController());

        PropertyValue[] params = new PropertyValue[]{new PropertyValue(), new PropertyValue(), new PropertyValue()};
        params[0].Name = "DataSourceName";
        params[0].Value = "Bibliography";
        params[1].Name = "CommandType";
        params[1].Value = new Integer(com.sun.star.sdb.CommandType.TABLE);
        params[2].Name = "Command";
        params[2].Value = "biblio";

        final PropertyValue[] fParams = params;

        shortWait();

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = secondController.getFrame().getContainerWindow();

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL, "", "AccessibleBrowseBox");

        log.println("ImplementationName: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);


        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {

                public void fireEvent() {
                    try {
                        xSelect.select(fParams);
                    } catch (com.sun.star.uno.Exception e) {
                        e.printStackTrace();
                    }
                }
            });

        return tEnv;
    }

    /**
     * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
     * reset</code> call.
     */
    private void shortWait() {
        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }
}
