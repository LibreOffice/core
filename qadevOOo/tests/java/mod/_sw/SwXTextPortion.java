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
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 *
 * @see com.sun.star.text.TextPortion
 *
 */
public class SwXTextPortion extends TestCase {
    XTextDocument xTextDoc;

    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory(tParam.getMSF());
        log.println("creating a textdocument");
        xTextDoc = SOF.createTextDoc(null);
    }

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
     * @see TestParameters * @see PrintWriter
     */
    @Override
    protected TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XInterface param = null;
        XPropertySet paraP = null;
        XPropertySet portP = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        // create testobject here

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println("inserting Strings");
        log.println("inserting ControlCharacter");

        for (int i = 0; i < 5; i++) {
            oText.insertString(oCursor, "Paragraph Number: " + i, false);
            oText.insertControlCharacter(oCursor, ControlCharacter.LINE_BREAK,
                    false);
            oText.insertString(
                    oCursor,
                    "The quick brown fox jumps over the lazy Dog: SwXParagraph\n",
                    false);
            oText.insertControlCharacter(oCursor, ControlCharacter.LINE_BREAK,
                    false);
            oText.insertString(
                    oCursor,
                    "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwXParagraph",
                    false);
            oText.insertControlCharacter(oCursor,
                    ControlCharacter.PARAGRAPH_BREAK, false);
        }

        // Enumeration
        XEnumerationAccess oEnumA = UnoRuntime.queryInterface(
                XEnumerationAccess.class, oText);
        XEnumeration oEnum = oEnumA.createEnumeration();

        int n = 0;
        while (oEnum.hasMoreElements()) {
            param = (XInterface) AnyConverter.toObject(new Type(
                    XInterface.class), oEnum.nextElement());
            log.println("Element Nr.: " + n);
            n++;
        }

        XEnumerationAccess oEnumP = UnoRuntime.queryInterface(
                XEnumerationAccess.class, param);
        XEnumeration oEnum2 = oEnumP.createEnumeration();
        oObj = (XInterface) AnyConverter.toObject(new Type(XInterface.class),
                oEnum2.nextElement());

        portP = UnoRuntime.queryInterface(XPropertySet.class, oObj);
        paraP = UnoRuntime.queryInterface(XPropertySet.class, param);

        log.println("creating a new environment for Paragraph object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("adding ObjRelation TRO for TextContent");
        tEnv.addObjRelation("TRO", Boolean.TRUE);

        log.println("adding ObjectRelation 'PARA' for CharacterProperties");
        tEnv.addObjRelation("PARA", paraP);

        log.println("adding ObjectRelation 'PORTION' for CharacterProperties");
        tEnv.addObjRelation("PORTION", portP);

        tEnv.addObjRelation("XTEXT", oText);

        return tEnv;
    } // finish method getTestEnvironment

} // finish class SwXTextPortion

