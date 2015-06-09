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

import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextViewCursorSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
/**
 *
 * initial description
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.beans.XPropertyState
 * @see com.sun.star.document.XDocumentInsertable
 * @see com.sun.star.style.CharacterProperties
 * @see com.sun.star.style.ParagraphProperties
 * @see com.sun.star.text.XPageCursor
 * @see com.sun.star.text.XParagraphCursor
 * @see com.sun.star.text.XSentenceCursor
 * @see com.sun.star.text.XTextCursor
 * @see com.sun.star.text.XTextRange
 * @see com.sun.star.text.XWordCursor
 * @see com.sun.star.util.XSortable
 * @see com.sun.star.view.XScreenCursor
 *
 */
public class SwXTextViewCursor extends TestCase {
    XTextDocument xTextDoc;

    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );

        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }


    /**
     *    creating a TestEnvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
     */
    @Override
    public TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // create testobject here
        oObj = xTextDoc.getCurrentController();

        XTextViewCursorSupplier oTVCSupp = UnoRuntime.queryInterface(XTextViewCursorSupplier.class, oObj);

        oObj = oTVCSupp.getViewCursor();

        log.println( "creating a new environment for TextViewCursor object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding TextDocument as mod relation to environment" );
        tEnv.addObjRelation("TEXTDOC", xTextDoc);
        tEnv.addObjRelation("XTEXT", xTextDoc.getText());

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class SwXTextViewCursor
