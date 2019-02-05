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
package mod._sw;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.WriterTools;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XViewSettingsSupplier;

public class SwAccessibleEndnoteView extends TestCase {

    XTextDocument xTextDoc = null;

    /**
    * Called to create an instance of <code>TestEnvironment</code>
    * with an object to test and related objects.
    * Inserts the created endnote to the document.
    * Changes zoom value to 10%(endnote must be in visible area of the document).
    * Obtains accessible object for the inserted endnote.
    *
    * @param Param test parameters
    * @param log writer to log information while testing
    *
    * @see TestEnvironment
    * @see #getTestEnvironment
    */
    @Override
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XInterface oEndnote = null;

        log.println( "Creating a test environment" );
        XMultiServiceFactory msf = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
        log.println("creating a endnote");
        oEndnote = UnoRuntime.queryInterface(XInterface.class,
                msf.createInstance("com.sun.star.text.Endnote"));

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println("inserting the footnote into text document");
        XTextContent xTC = UnoRuntime.queryInterface(XTextContent.class, oEndnote);
        oText.insertTextContent(oCursor, xTC, false);

        XController xController = xTextDoc.getCurrentController();
        XViewSettingsSupplier xViewSetSup = UnoRuntime.queryInterface(XViewSettingsSupplier.class,
        xController);
        XPropertySet xPropSet = xViewSetSup.getViewSettings();

        //change zoom value to 20%
        //footer should be in the vissible area of the document
        xPropSet.setPropertyValue("ZoomValue", Short.valueOf("20"));

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xTextDoc);

        XWindow xWindow = AccessibilityTools.getCurrentWindow(aModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.END_NOTE);

        log.println("ImplementationName " + utils.getImplName(oObj));
        AccessibilityTools.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XPropertySet PropSet = xViewSetSup.getViewSettings();

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    try {
                        //change zoom value to 21%
                        PropSet.setPropertyValue("ZoomValue", Short.valueOf("21"));
                        //and back to 20%
                        PropSet.setPropertyValue("ZoomValue", Short.valueOf("20"));
                    } catch ( com.sun.star.lang.WrappedTargetException e ) {

                    }  catch ( com.sun.star.lang.IllegalArgumentException e ) {

                    } catch ( com.sun.star.beans.PropertyVetoException e ) {

                    } catch ( com.sun.star.beans.UnknownPropertyException e ) {

                    }
                }
            });

        return tEnv;

    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes text document.
    * @param Param test parameters
    * @param log writer to log information while testing
    */
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("dispose text document");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        log.println( "creating a text document" );
        xTextDoc = WriterTools.createTextDoc(Param.getMSF());
    }
}
