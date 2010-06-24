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
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XTableRows;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


public class SwXTextTableRow extends TestCase {
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

    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        XTextTable oTable = null;



        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF() );

        try {
         log.println("creating a texttable");
            oTable = (XTextTable) SOF.createTextTable(xTextDoc,5,5);
        } catch( Exception e ) {
            e.printStackTrace(log);
        }

        XText oText = xTextDoc.getText();

        XTextCursor oCursor = oText.createTextCursor();

        try {
            log.println("inserting texttable");
            oText.insertTextContent(oCursor, oTable, false);
        } catch (Exception e) {
            log.println("Exception!");
        }

        try {
         log.println("getting table row");
            XTableRows oTRn = oTable.getRows();
            XIndexAccess oIA = (XIndexAccess) UnoRuntime.queryInterface
                (XIndexAccess.class,oTRn);
            oObj = (XPropertySet) AnyConverter.toObject(
                    new Type(XPropertySet.class),oIA.getByIndex(1));
        } catch( Exception e ) {
            e.printStackTrace(log);
        }

        log.println( "creating a new environment for TextTableRow object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding TextDocument as mod relation to environment" );
        tEnv.addObjRelation("TEXTDOC", xTextDoc);


        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXTextTableRow

