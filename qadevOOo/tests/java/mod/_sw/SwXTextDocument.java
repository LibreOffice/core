/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SwXTextDocument.java,v $
 * $Revision: 1.9 $
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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;

import util.SOfficeFactory;
import util.WriterTools;


public class SwXTextDocument extends TestCase {
    XTextDocument xTextDoc;
    XTextDocument xSecondTextDoc;

    protected void initialize(TestParameters tParam, PrintWriter log) {
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");
        DesktopTools.closeDoc(xSecondTextDoc);
        DesktopTools.closeDoc(xTextDoc);
    }

    /**
     *    creating a Testenvironment for the interfaces to be tested
     */
    public synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                              PrintWriter log)
        throws StatusException {
        TestEnvironment tEnv = null;
        XTextTable the_table = null;

        try {
            log.println("creating a textdocument");
            xTextDoc = WriterTools.createTextDoc(
                               (XMultiServiceFactory) Param.getMSF());

            XText oText = xTextDoc.getText();
            XTextCursor oCursor = oText.createTextCursor();

            log.println("inserting some lines");

            try {
                for (int i = 0; i < 5; i++) {
                    oText.insertString(oCursor, "Paragraph Number: " + i,
                                       false);
                    oText.insertString(oCursor,
                                       " The quick brown fox jumps over the lazy Dog: SwXParagraph",
                                       false);
                    oText.insertControlCharacter(oCursor,
                                                 ControlCharacter.PARAGRAPH_BREAK,
                                                 false);
                    oText.insertString(oCursor,
                                       "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwXParagraph",
                                       false);
                    oText.insertControlCharacter(oCursor,
                                                 ControlCharacter.PARAGRAPH_BREAK,
                                                 false);
                    oText.insertControlCharacter(oCursor,
                                                 ControlCharacter.LINE_BREAK,
                                                 false);
                }
                            for (int i = 0; i < 11; i++) {
                oText.insertString(oCursor, "xTextDoc ", false);

            }
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                e.printStackTrace(log);
                throw new StatusException("Couldn't insert lines", e);
            }

            //insert two sections parent and child
            XMultiServiceFactory oDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                                   XMultiServiceFactory.class,
                                                   xTextDoc);

            XInterface oTS;
            XTextSection xTS;

            XTextSectionsSupplier oTSSupp = (XTextSectionsSupplier) UnoRuntime.queryInterface(
                                                    XTextSectionsSupplier.class,
                                                    xTextDoc);
            XNameAccess oTSSuppName = oTSSupp.getTextSections();

            oTS = (XInterface) oDocMSF.createInstance(
                          "com.sun.star.text.TextSection");

            XTextContent oTSC = (XTextContent) UnoRuntime.queryInterface(
                                        XTextContent.class, oTS);
            oText.insertTextContent(oCursor, oTSC, false);

            XWordCursor oWordC = (XWordCursor) UnoRuntime.queryInterface(
                                         XWordCursor.class, oCursor);
            oCursor.setString("End of TextSection");
            oCursor.gotoStart(false);
            oCursor.setString("Start of TextSection ");
            oWordC.gotoEndOfWord(false);

            XInterface oTS2 = (XInterface) oDocMSF.createInstance(
                                      "com.sun.star.text.TextSection");
            oTSC = (XTextContent) UnoRuntime.queryInterface(XTextContent.class,
                                                            oTS2);
            oText.insertTextContent(oCursor, oTSC, false);

            XIndexAccess oTSSuppIndex = (XIndexAccess) UnoRuntime.queryInterface(
                                                XIndexAccess.class,
                                                oTSSuppName);
            log.println(
                    "getting a TextSection with the XTextSectionSupplier()");
            xTS = (XTextSection) UnoRuntime.queryInterface(XTextSection.class,
                                                           oTSSuppIndex.getByIndex(0));

            XNamed xTSName = (XNamed) UnoRuntime.queryInterface(XNamed.class,
                                                                xTS);
            xTSName.setName("SwXTextSection");

            log.println("    adding TextTable");
            the_table = SOfficeFactory.createTextTable(xTextDoc, 6, 4);

            XNamed the_name = (XNamed) UnoRuntime.queryInterface(XNamed.class,
                                                                 the_table);
            the_name.setName("SwXTextDocument");
            SOfficeFactory.insertTextContent(xTextDoc,
                                             (XTextContent) the_table);

            log.println("    adding ReferenceMark");

            XInterface aMark = (XInterface) oDocMSF.createInstance(
                                       "com.sun.star.text.ReferenceMark");
            the_name = (XNamed) UnoRuntime.queryInterface(XNamed.class, aMark);
            the_name.setName("SwXTextDocument");

            XTextContent oTC = (XTextContent) UnoRuntime.queryInterface(
                                       XTextContent.class, aMark);
            SOfficeFactory.insertTextContent(xTextDoc, (XTextContent) oTC);

            log.println("    adding TextGraphic");
            WriterTools.insertTextGraphic(xTextDoc, oDocMSF, 5200, 4200, 4400,
                                          4000, "space-metal.jpg",
                                          "SwXTextDocument");

            log.println("    adding EndNote");

            XInterface aEndNote = (XInterface) oDocMSF.createInstance(
                                          "com.sun.star.text.Endnote");
            oTC = (XTextContent) UnoRuntime.queryInterface(XTextContent.class,
                                                           aEndNote);
            SOfficeFactory.insertTextContent(xTextDoc, (XTextContent) oTC);

            log.println("    adding Bookmark");
            SOfficeFactory.insertTextContent(xTextDoc,
                                             (XTextContent) SOfficeFactory.createBookmark(
                                                     xTextDoc));

            log.println("    adding DocumentIndex");
            oTC = SOfficeFactory.createIndex(xTextDoc,
                                             "com.sun.star.text.DocumentIndex");
            SOfficeFactory.insertTextContent(xTextDoc, (XTextContent) oTC);

            log.println("    adding TextFrame");

            XTextFrame frame = SOfficeFactory.createTextFrame(xTextDoc, 500,
                                                              500);
            oTC = (XTextContent) UnoRuntime.queryInterface(XTextContent.class,
                                                           frame);
            SOfficeFactory.insertTextContent(xTextDoc, (XTextContent) oTC);

            log.println("creating a second textdocument");
            xSecondTextDoc = WriterTools.createTextDoc(
                                     (XMultiServiceFactory) Param.getMSF());
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }

        if (xTextDoc != null) {
            log.println("Creating instance...");

            XText oText = xTextDoc.getText();
            XTextCursor oTextCursor = oText.createTextCursor();

            for (int i = 0; i < 11; i++) {
                oText.insertString(oTextCursor, "xTextDoc ", false);
            }

            tEnv = new TestEnvironment(xTextDoc);
        } else {
            log.println("Failed to create instance.");

            return tEnv;
        }

        XModel model1 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                           xTextDoc);
        XModel model2 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                           xSecondTextDoc);

        XController cont1 = model1.getCurrentController();
        XController cont2 = model2.getCurrentController();

        cont1.getFrame().setName("cont1");
        cont2.getFrame().setName("cont2");

        XSelectionSupplier sel = (XSelectionSupplier) UnoRuntime.queryInterface(
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
