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

import com.sun.star.lang.XMultiServiceFactory;
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

    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }


    /**
     *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
     */
    public synchronized TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // create testobject here
        oObj = xTextDoc.getCurrentController();

        XTextViewCursorSupplier oTVCSupp = (XTextViewCursorSupplier)
            UnoRuntime.queryInterface(XTextViewCursorSupplier.class, oObj);

        oObj = oTVCSupp.getViewCursor();

        log.println( "creating a new environment for TextViewCursor object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding TextDocument as mod relation to environment" );
        tEnv.addObjRelation("TEXTDOC", xTextDoc);
        tEnv.addObjRelation("XTEXT", xTextDoc.getText());

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class SwXTextViewCursor
