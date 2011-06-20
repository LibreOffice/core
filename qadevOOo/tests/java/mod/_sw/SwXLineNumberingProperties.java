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
import util.dbg;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
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
 * threads concurently.
 * @see com.sun.star.text.LineNumberingProperties
 * @see ifc.text._LineNumberingProperties
 */
public class SwXLineNumberingProperties extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes text document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }


    /**
    * Creating a Testenvironment for the interfaces to be tested. After inserting
    * string and control character to the text document, line numbering
    * properties are gotten using <code>XLineNumberingProperties</code> interface.
    */
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

        XLineNumberingProperties oLNP = (XLineNumberingProperties)
                UnoRuntime.queryInterface(XLineNumberingProperties.class,xTextDoc);
        XPropertySet lineNumProps = oLNP.getLineNumberingProperties();
        dbg.printPropertiesNames(lineNumProps);
        TestEnvironment tEnv = new TestEnvironment(lineNumProps);
        return tEnv;

    } // finish method getTestEnvironment

}    // finish class SwXLineNumberingProperties

