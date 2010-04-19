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
import util.InstCreator;
import util.ParagraphDsc;
import util.SOfficeFactory;
import util.TableDsc;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFrame;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XEnumerationAccess
 * @see com.sun.star.text.XSimpleText
 * @see com.sun.star.text.XText
 * @see com.sun.star.text.XTextRange
 * @see com.sun.star.text.XTextRangeMover
 *
 */
public class SwXTextFrameText extends TestCase {
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
            (TestParameters tParam, PrintWriter log ) {

        XInterface oObj = null;
        XTextFrame oFrame1 = null;
        XPropertySet oPropSet = null;
        XText oText = null;
        XTextCursor oCursor = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        // create testobject here
        ////////////////////////////////////

        try {
            oFrame1 = SOF.createTextFrame(xTextDoc, 500, 500);
            oPropSet = (XPropertySet)UnoRuntime.queryInterface
                (XPropertySet.class, oFrame1 );
            //AnchorTypes: 0 = paragraph, 1 = as char, 2 = page,
            // 3 = frame/paragraph 4= at char
            oPropSet.setPropertyValue("AnchorType", new Integer(2));
            oText = xTextDoc.getText();
                    oCursor = oText.createTextCursor();

            log.println( "inserting Frame1" );
            oText.insertTextContent(oCursor,oFrame1, false);

            } catch (Exception Ex ) {
            Ex.printStackTrace(log);
            throw new StatusException("Couldn't insert TextFrame ", Ex);
        }

        XText oFText = (XText)UnoRuntime.queryInterface(XText.class, oFrame1);
        XTextCursor oFCursor = oFText.createTextCursor();
        oFText.insertString(oFCursor, "SwXTextFrameText", false);

        oObj = oFText.getText();

        log.println( "creating a new environment for TextFrameText object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding TextDocument as mod relation to environment" );
        tEnv.addObjRelation("TEXT", (XText) oObj);

        log.println( "adding InstDescriptor object" );
        TableDsc tDsc = new TableDsc( 6, 4 );

        log.println( "adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xTextDoc, tDsc ) );

        log.println( "    adding Paragraph" );
        ParagraphDsc pDsc = new ParagraphDsc();
        tEnv.addObjRelation( "PARA", new InstCreator( xTextDoc, pDsc ) );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXTextFrameText

