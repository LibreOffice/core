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
            oFrame1 = SOF.createTextFrame(xTextDoc, 500, 500);
            oFrame2 = SOF.createTextFrame(xTextDoc, 1500, 1500);
            oPropSet = (XPropertySet) UnoRuntime.queryInterface(
                               XPropertySet.class, oFrame1);


            // AnchorTypes: 0 = paragraph, 1 = as char, 2 = page,
            // 3 = frame/paragraph 4= at char
            oPropSet.setPropertyValue("AnchorType",
                                      TextContentAnchorType.AS_CHARACTER);
            oText = xTextDoc.getText();
            oCursor = oText.createTextCursor();

            log.println("inserting Frame1");

            XTextContent the_content = (XTextContent) UnoRuntime.queryInterface(
                                               XTextContent.class, oFrame1);
            oText.insertTextContent(oCursor, the_content, true);

            log.println("inserting Frame2");
            the_content = (XTextContent) UnoRuntime.queryInterface(
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
                            (XTextContent) UnoRuntime.queryInterface(
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
