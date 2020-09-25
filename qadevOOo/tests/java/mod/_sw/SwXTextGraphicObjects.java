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
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextGraphicObjectsSupplier;
import com.sun.star.graphic.XGraphic;
import com.sun.star.graphic.XGraphicProvider;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XComponentContext;

public class SwXTextGraphicObjects extends TestCase {

    XTextDocument xTextDoc;

    /**
     * in general this method creates a testdocument
     *
     * @param tParam
     *            class which contains additional test parameters
     * @param log
     *            class to log the test state and result
     *
     *
     * @see TestParameters
     * @see PrintWriter
     *
     */
    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory(tParam.getMSF());
        log.println("creating a textdoc");
        xTextDoc = SOF.createTextDoc(null);
    }

    /**
     * in general this method disposes the testenvironment and document
     *
     * @param tParam
     *            class which contains additional test parameters
     * @param log
     *            class to log the test state and result
     *
     *
     * @see TestParameters
     * @see PrintWriter
     *
     */
    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     * creating a TestEnvironment for the interfaces to be tested
     *
     * @param tParam
     *            class which contains additional test parameters
     * @param log
     *            class to log the test state and result
     *
     * @return Status class
     *
     * @see TestParameters
     * @see PrintWriter
     */
    @Override
    public TestEnvironment createTestEnvironment(TestParameters tParam,
            PrintWriter log) throws Exception {

        XInterface oObj = null;
        Object oGObject = null;
        SOfficeFactory SOF = SOfficeFactory.getFactory(tParam.getMSF());

        oGObject = SOF.createInstance(xTextDoc,
                "com.sun.star.text.GraphicObject");

        oObj = (XInterface) oGObject;

        XText the_text = xTextDoc.getText();
        XTextCursor the_cursor = the_text.createTextCursor();
        XTextContent the_content = UnoRuntime.queryInterface(
                XTextContent.class, oObj);

        log.println("inserting graphic");
        the_text.insertTextContent(the_cursor, the_content, true);

        log.println("adding graphic");

        XComponentContext xContext = tParam.getComponentContext();

        XGraphicProvider xGraphicProvider = UnoRuntime.queryInterface(XGraphicProvider.class,
            xContext.getServiceManager().createInstanceWithContext("com.sun.star.graphic.GraphicProvider", xContext));

        String fullURL = util.utils.getFullTestURL("space-metal.jpg");

        PropertyValue[] aMediaProps = new PropertyValue[] { new PropertyValue() };
        aMediaProps[0].Name = "URL";
        aMediaProps[0].Value = fullURL;

        XGraphic xGraphic = UnoRuntime.queryInterface(XGraphic.class,
                                xGraphicProvider.queryGraphic(aMediaProps));

        XPropertySet oProps = UnoRuntime.queryInterface(XPropertySet.class,
                oObj);
        oProps.setPropertyValue("Graphic", xGraphic);
        oProps.setPropertyValue("HoriOrientPosition", Integer.valueOf(5500));
        oProps.setPropertyValue("VertOrientPosition", Integer.valueOf(4200));
        oProps.setPropertyValue("Width", Integer.valueOf(4400));
        oProps.setPropertyValue("Height", Integer.valueOf(4000));

        XTextGraphicObjectsSupplier xTGS = UnoRuntime.queryInterface(
                XTextGraphicObjectsSupplier.class, xTextDoc);
        oObj = xTGS.getGraphicObjects();

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;

    } // finish method getTestEnvironment

} // finish class SwXTextGraphicObjects
