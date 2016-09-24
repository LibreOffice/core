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
import java.util.ArrayList;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XPropertySet;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XParagraphCursor;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.TextCursor</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::XTextCursor</code></li>
 *  <li> <code>com::sun::star::text::XWordCursor</code></li>
 *  <li> <code>com::sun::star::style::CharacterPropertiesComplex</code></li>
 *  <li> <code>com::sun::star::text::XTextRange</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::container::XContentEnumerationAccess</code></li>
 *  <li> <code>com::sun::star::beans::XPropertyState</code></li>
 *  <li> <code>com::sun::star::style::CharacterProperties</code></li>
 *  <li> <code>com::sun::star::text::XSentenceCursor</code></li>
 *  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
 *  <li> <code>com::sun::star::text::XParagraphCursor</code></li>
 *  <li> <code>com::sun::star::document::XDocumentInsertable</code></li>
 *  <li> <code>com::sun::star::util::XSortable</code></li>
 *  <li> <code>com::sun::star::style::CharacterPropertiesAsian</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.text.XTextCursor
 * @see com.sun.star.text.XWordCursor
 * @see com.sun.star.style.CharacterPropertiesComplex
 * @see com.sun.star.text.XTextRange
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.container.XContentEnumerationAccess
 * @see com.sun.star.beans.XPropertyState
 * @see com.sun.star.style.CharacterProperties
 * @see com.sun.star.text.XSentenceCursor
 * @see com.sun.star.style.ParagraphProperties
 * @see com.sun.star.text.XParagraphCursor
 * @see com.sun.star.document.XDocumentInsertable
 * @see com.sun.star.util.XSortable
 * @see com.sun.star.style.CharacterPropertiesAsian
 * @see ifc.text._XTextCursor
 * @see ifc.text._XWordCursor
 * @see ifc.style._CharacterPropertiesComplex
 * @see ifc.text._XTextRange
 * @see ifc.beans._XPropertySet
 * @see ifc.container._XContentEnumerationAccess
 * @see ifc.beans._XPropertyState
 * @see ifc.style._CharacterProperties
 * @see ifc.text._XSentenceCursor
 * @see ifc.style._ParagraphProperties
 * @see ifc.text._XParagraphCursor
 * @see ifc.document._XDocumentInsertable
 * @see ifc.util._XSortable
 * @see ifc.style._CharacterPropertiesAsian
 */
public class SwXTextCursor extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());
        log.println("creating a textdocument");
        xTextDoc = SOF.createTextDoc(null);
    }

    /**
    * Disposes text document.
    */
    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested. After major
    * text of text document is obtained, text cursor is created and several
    * paragraphs within the text are inserted to a text document. Finally,
    * text cursor is returned as a test component.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XTEXT'</code> for
    *      {@link ifc.text._XTextRange} : major text of text document</li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;

        log.println("creating a test environment");


        // get the bodytext of textdocument here
        log.println("getting the TextCursor");

        final XSimpleText aText = xTextDoc.getText();
        final XTextCursor textCursor = aText.createTextCursor();
        oObj = textCursor;

        log.println("inserting some text");

        try {
            for (int i = 0; i < 3; i++) {
                aText.insertString(textCursor, "" + (3 - i), false);

                for (int j = 0; j < 5; j++) {
                    aText.insertString(textCursor, "XTextCursor,XTextCursor",
                                       false);
                    aText.insertString(textCursor, "The quick brown fox ",
                                       false);
                    aText.insertString(textCursor, "jumps over the lazy dog ",
                                       false);
                }

                aText.insertControlCharacter(textCursor,
                                             ControlCharacter.PARAGRAPH_BREAK,
                                             false);
                aText.insertControlCharacter(textCursor,
                                             ControlCharacter.LINE_BREAK,
                                             false);
            }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Error, insert text to text document.");
            e.printStackTrace(log);
        }

        log.println("creating a new environment for SwXTextCursor object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("XTEXT", xTextDoc.getText());

        XPropertySet xCursorProp = UnoRuntime.queryInterface(
                                           XPropertySet.class, oObj);
        tEnv.addObjRelation("PropertyNames", getPropertyNames(xCursorProp));

        //Adding relation for util.XSortable
        final XParagraphCursor paragrCursor = UnoRuntime.queryInterface(
                                                      XParagraphCursor.class,
                                                      oObj);
        final PrintWriter finalLog = log;

        tEnv.addObjRelation("SORTCHECKER",
                            new ifc.util._XSortable.XSortChecker() {
            PrintWriter out = finalLog;

            public void setPrintWriter(PrintWriter log) {
                out = log;
            }

            public void prepareToSort() {
                textCursor.gotoEnd(false);

                try {
                    aText.insertControlCharacter(textCursor,
                                                 ControlCharacter.PARAGRAPH_BREAK,
                                                 false);
                    aText.insertString(textCursor, "4", false);
                    aText.insertControlCharacter(textCursor,
                                                 ControlCharacter.PARAGRAPH_BREAK,
                                                 false);
                    aText.insertString(textCursor, "b", false);
                    aText.insertControlCharacter(textCursor,
                                                 ControlCharacter.PARAGRAPH_BREAK,
                                                 false);
                    aText.insertString(textCursor, "3", false);
                    aText.insertControlCharacter(textCursor,
                                                 ControlCharacter.PARAGRAPH_BREAK,
                                                 false);
                    aText.insertString(textCursor, "a", false);
                    aText.insertControlCharacter(textCursor,
                                                 ControlCharacter.PARAGRAPH_BREAK,
                                                 false);
                    aText.insertString(textCursor, "23", false);
                    aText.insertControlCharacter(textCursor,
                                                 ControlCharacter.PARAGRAPH_BREAK,
                                                 false);
                    aText.insertString(textCursor, "ab", false);
                    aText.insertControlCharacter(textCursor,
                                                 ControlCharacter.PARAGRAPH_BREAK,
                                                 false);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    out.println("Unexpected exception:" + e);
                }

                out.println(
                        "Preparing cursor to sorting. Text before sorting:");
                paragrCursor.gotoEnd(true);

                for (int i = 0; i < 6; i++) {
                    paragrCursor.gotoPreviousParagraph(true);
                }

                out.println(textCursor.getString());
            }

            public boolean checkSort(boolean isSortNumbering,
                                     boolean isSortAscending) {
                out.println("Sort checking...");

                String ls = System.getProperty("line.separator");

                String text = paragrCursor.getString();
                out.println("Text after sorting:\n" + text);

                boolean res = false;

                if (isSortNumbering) {
                    if (isSortAscending) {
                        res = text.endsWith(ls+"3"+ls+"4"+ls+"23");

                        if (!res) {
                            out.println("Text must ends by:\n" + "\r\n3\r\n4\r\n23\r\n");
                        }
                    } else {
                        res = text.startsWith("23"+ls+"4"+ls+"3"+ls);

                        if (!res) {
                            out.println("Text must starts with:\n" + "23\r\n4\r\n3\r\n");
                        }
                    }
                } else {
                    if (isSortAscending) {
                        res = text.equals(ls+"23"+ls+"3"+ls+"4"+ls+"a"+ls+"ab"+ls+"b");

                        if (!res) {
                            out.println("Text must be equal to:\n" + "\r\n23\r\n3\r\n4\r\na\r\nab\r\nb\r\n");
                        }
                    } else {
                        res = text.endsWith("b"+ls+"ab"+ls+"a"+ls+"4"+ls+"3"+ls+"23"+ls);

                        if (!res) {
                            out.println("Text must be equal to:\n" + "b\r\nab\r\na\r\n4\r\n3\r\n23\r\n");
                        }
                    }
                }

                if (res) {
                    out.println("Sorted correctly");
                }

                return res;
            }
        });

        return tEnv;
    } // finish method getTestEnvironment

    public String[] getPropertyNames(XPropertySet props) {
        Property[] the_props = props.getPropertySetInfo().getProperties();
        ArrayList<String> names = new ArrayList<String>();

        for (int i = 0; i < the_props.length; i++) {
            boolean isWritable = ((the_props[i].Attributes & PropertyAttribute.READONLY) == 0);

            if (isWritable) {
                names.add(the_props[i].Name);
            }
        }

        return names.toArray(new String[names.size()]);
    }
} // finish class SwXTextCursor
