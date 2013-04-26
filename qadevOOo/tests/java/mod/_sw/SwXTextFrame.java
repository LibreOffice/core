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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.TextContentAnchorType;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFrame;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 *
 * initial description
 * @see com.sun.star.text.XText
 *
 */
public class SwXTextFrame extends TestCase {
    XTextDocument xTextDoc;

    protected void initialize(TestParameters tParam, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a textdocument");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     *    creating a Testenvironment for the interfaces to be tested
     */
    public synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                              PrintWriter log) {
        XInterface oObj = null;
        XTextFrame oFrame1 = null;
        XTextFrame oFrame2 = null;
        XPropertySet oPropSet = null;
        XText oText = null;
        XTextCursor oCursor = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory) Param.getMSF());


        // creating Frames
        log.println("creating Frames");


        Object instance = null;

        try {
            oFrame1 = SOfficeFactory.createTextFrame(xTextDoc, 500, 500);
            oFrame2 = SOfficeFactory.createTextFrame(xTextDoc, 1500, 1500);
            oPropSet = UnoRuntime.queryInterface(
                               XPropertySet.class, oFrame1);


            // AnchorTypes: 0 = paragraph, 1 = as char, 2 = page,
            // 3 = frame/paragraph 4= at char
            oPropSet.setPropertyValue("AnchorType",
                                      TextContentAnchorType.AS_CHARACTER);
            oText = xTextDoc.getText();
            oCursor = oText.createTextCursor();

            log.println("inserting Frame1");

            XTextContent the_content = UnoRuntime.queryInterface(
                                               XTextContent.class, oFrame1);
            oText.insertTextContent(oCursor, the_content, true);

            log.println("inserting Frame2");
            the_content = UnoRuntime.queryInterface(
                                  XTextContent.class, oFrame2);
            oText.insertTextContent(oCursor, the_content, true);

            XText oFrameText = oFrame1.getText();
            oFrameText.insertString(oFrameText.getStart(), "The FrameText",
                                    true);

            instance = SOF.createInstance(xTextDoc,
                                          "com.sun.star.text.TextFrame");
        } catch (Exception Ex) {
            Ex.printStackTrace(log);
            throw new StatusException("Couldn't insert TextFrame ", Ex);
        }

        oObj = oFrame1;

        log.println("creating a new environment for TextFrame object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("CONTENT",
                            UnoRuntime.queryInterface(
                                    XTextContent.class, instance));
        tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

        log.println("adding ObjRelation for XShape " +
                    "(get/setPosition won't work there)");
        tEnv.addObjRelation("NoPos", "SwXTextFrame");
        tEnv.addObjRelation("NoSetSize", "SwXTextFrame");

        tEnv.addObjRelation("TextFrame", oFrame2);

        return tEnv;
    } // finish method getTestEnvironment
} // finish class SwXTextFrame
