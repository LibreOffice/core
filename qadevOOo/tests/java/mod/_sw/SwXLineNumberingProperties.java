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
import util.dbg;

import com.sun.star.beans.XPropertySet;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XLineNumberingProperties;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.LineNumberingProperties</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::LineNumberingProperties</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.text.LineNumberingProperties
 * @see ifc.text._LineNumberingProperties
 */
public class SwXLineNumberingProperties extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    /**
    * Disposes text document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }


    /**
    * Creating a TestEnvironment for the interfaces to be tested. After inserting
    * string and control character to the text document, line numbering
    * properties are gotten using <code>XLineNumberingProperties</code> interface.
    */
    @Override
    public TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {

        // insert some Text
        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();
        try {
            for (int i=0; i<5; i++) {
                oText.insertString(oCursor, "The quick brown fox jumps "+
                    "over the lazy Dog", false);
                oText.insertControlCharacter(oCursor,
                    ControlCharacter.PARAGRAPH_BREAK, false );
            }
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            log.println("Exception occurred: " + e);
        }

        XLineNumberingProperties oLNP = UnoRuntime.queryInterface(XLineNumberingProperties.class,xTextDoc);
        XPropertySet lineNumProps = oLNP.getLineNumberingProperties();
        dbg.printPropertiesNames(lineNumProps);
        TestEnvironment tEnv = new TestEnvironment(lineNumProps);
        return tEnv;

    } // finish method getTestEnvironment

}    // finish class SwXLineNumberingProperties

