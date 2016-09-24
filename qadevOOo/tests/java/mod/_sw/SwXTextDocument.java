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

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFrame;
import com.sun.star.text.XTextSection;
import com.sun.star.text.XTextSectionsSupplier;
import com.sun.star.text.XTextTable;
import com.sun.star.text.XWordCursor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XSelectionSupplier;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;

import util.SOfficeFactory;
import util.WriterTools;

public class SwXTextDocument extends TestCase {
    XTextDocument xTextDoc;
    XTextDocument xSecondTextDoc;

    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");
        DesktopTools.closeDoc(xSecondTextDoc);
        DesktopTools.closeDoc(xTextDoc);
    }

    /**
     * creating a TestEnvironment for the interfaces to be tested
     */
    @Override
    public TestEnvironment createTestEnvironment(
            TestParameters Param, PrintWriter log) throws Exception {
        TestEnvironment tEnv = null;
        XTextTable the_table = null;

        log.println("creating a textdocument");
        xTextDoc = WriterTools.createTextDoc(Param.getMSF());

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println("inserting some lines");

        for (int i = 0; i < 5; i++) {
            oText.insertString(oCursor, "Paragraph Number: " + i, false);
            oText.insertString(
                    oCursor,
                    " The quick brown fox jumps over the lazy Dog: SwXParagraph",
                    false);
            oText.insertControlCharacter(oCursor,
                    ControlCharacter.PARAGRAPH_BREAK, false);
            oText.insertString(
                    oCursor,
                    "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwXParagraph",
                    false);
            oText.insertControlCharacter(oCursor,
                    ControlCharacter.PARAGRAPH_BREAK, false);
            oText.insertControlCharacter(oCursor, ControlCharacter.LINE_BREAK,
                    false);
        }
        for (int i = 0; i < 11; i++) {
            oText.insertString(oCursor, "xTextDoc ", false);

        }

        // insert two sections parent and child
        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(
                XMultiServiceFactory.class, xTextDoc);

        XInterface oTS;
        XTextSection xTS;

        XTextSectionsSupplier oTSSupp = UnoRuntime.queryInterface(
                XTextSectionsSupplier.class, xTextDoc);
        XNameAccess oTSSuppName = oTSSupp.getTextSections();

        oTS = (XInterface) oDocMSF
                .createInstance("com.sun.star.text.TextSection");

        XTextContent oTSC = UnoRuntime.queryInterface(XTextContent.class, oTS);
        oText.insertTextContent(oCursor, oTSC, false);

        XWordCursor oWordC = UnoRuntime.queryInterface(XWordCursor.class,
                oCursor);
        oCursor.setString("End of TextSection");
        oCursor.gotoStart(false);
        oCursor.setString("Start of TextSection ");
        oWordC.gotoEndOfWord(false);

        XInterface oTS2 = (XInterface) oDocMSF
                .createInstance("com.sun.star.text.TextSection");
        oTSC = UnoRuntime.queryInterface(XTextContent.class, oTS2);
        oText.insertTextContent(oCursor, oTSC, false);

        XIndexAccess oTSSuppIndex = UnoRuntime.queryInterface(
                XIndexAccess.class, oTSSuppName);
        log.println("getting a TextSection with the XTextSectionSupplier()");
        xTS = UnoRuntime.queryInterface(XTextSection.class,
                oTSSuppIndex.getByIndex(0));

        XNamed xTSName = UnoRuntime.queryInterface(XNamed.class, xTS);
        xTSName.setName("SwXTextSection");

        log.println("    adding TextTable");
        the_table = SOfficeFactory.createTextTable(xTextDoc, 6, 4);

        XNamed the_name = UnoRuntime.queryInterface(XNamed.class, the_table);
        the_name.setName("SwXTextDocument");
        SOfficeFactory.insertTextContent(xTextDoc, the_table);

        log.println("    adding ReferenceMark");

        XInterface aMark = (XInterface) oDocMSF
                .createInstance("com.sun.star.text.ReferenceMark");
        the_name = UnoRuntime.queryInterface(XNamed.class, aMark);
        the_name.setName("SwXTextDocument");

        XTextContent oTC = UnoRuntime.queryInterface(XTextContent.class, aMark);
        SOfficeFactory.insertTextContent(xTextDoc, oTC);

        log.println("    adding TextGraphic");
        WriterTools.insertTextGraphic(xTextDoc, oDocMSF, 5200, 4200, 4400,
                4000, "space-metal.jpg", "SwXTextDocument");

        log.println("    adding EndNote");

        XInterface aEndNote = (XInterface) oDocMSF
                .createInstance("com.sun.star.text.Endnote");
        oTC = UnoRuntime.queryInterface(XTextContent.class, aEndNote);
        SOfficeFactory.insertTextContent(xTextDoc, oTC);

        log.println("    adding Bookmark");
        SOfficeFactory.insertTextContent(xTextDoc,
                SOfficeFactory.createBookmark(xTextDoc));

        log.println("    adding DocumentIndex");
        oTC = SOfficeFactory.createIndex(xTextDoc,
                "com.sun.star.text.DocumentIndex");
        SOfficeFactory.insertTextContent(xTextDoc, oTC);

        log.println("    adding TextFrame");

        XTextFrame frame = SOfficeFactory.createTextFrame(xTextDoc, 500, 500);
        oTC = UnoRuntime.queryInterface(XTextContent.class, frame);
        SOfficeFactory.insertTextContent(xTextDoc, oTC);

        log.println("creating a second textdocument");
        xSecondTextDoc = WriterTools.createTextDoc(Param.getMSF());

        XText oText2 = xTextDoc.getText();
        XTextCursor oTextCursor = oText2.createTextCursor();

        for (int i = 0; i < 11; i++) {
            oText2.insertString(oTextCursor, "xTextDoc ", false);
        }

        tEnv = new TestEnvironment(xTextDoc);

        XModel model1 = UnoRuntime.queryInterface(XModel.class, xTextDoc);
        XModel model2 = UnoRuntime.queryInterface(XModel.class, xSecondTextDoc);

        XController cont1 = model1.getCurrentController();
        XController cont2 = model2.getCurrentController();

        cont1.getFrame().setName("cont1");
        cont2.getFrame().setName("cont2");

        XSelectionSupplier sel = UnoRuntime.queryInterface(
                XSelectionSupplier.class, cont1);

        log.println("Adding SelectionSupplier and Shape to select for XModel");
        tEnv.addObjRelation("SELSUPP", sel);
        tEnv.addObjRelation("TOSELECT", the_table);

        log.println("adding Controller as ObjRelation for XModel");
        tEnv.addObjRelation("CONT1", cont1);
        tEnv.addObjRelation("CONT2", cont2);

        return tEnv;
    }
} // finish class SwXTextDocument
