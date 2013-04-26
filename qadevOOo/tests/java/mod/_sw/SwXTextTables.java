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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
import com.sun.star.text.XTextTablesSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XNameAccess
 *
 */
public class SwXTextTables extends TestCase {
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
        int nRow = 4;
        int nCol = 5;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        // create testobject here
        TestEnvironment tEnv = null;
        XTextTable oTable = null;
        try {
            oTable = SOfficeFactory.createTextTable( xTextDoc );
            SOfficeFactory.insertTextContent(xTextDoc, oTable );
        }
        catch( Exception uE ) {
            uE.printStackTrace( log );
            throw new StatusException("Couldn't create TextTable : "
                    + uE.getMessage(), uE);
        }

        // Number two
        XTextTable oTable2 = null;
        try {
            oTable2 = SOfficeFactory.createTextTable( xTextDoc );
            SOfficeFactory.insertTextContent(xTextDoc, oTable2 );
        }
        catch( Exception uE ) {
            uE.printStackTrace( log );
            throw new StatusException("Couldn't create TextTable two: "
                    + uE.getMessage(), uE);
        }

        XMultiServiceFactory msf = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
        XTextTablesSupplier oTTSupp = UnoRuntime.queryInterface(XTextTablesSupplier.class, msf);
        oObj = oTTSupp.getTextTables();

        if ( oTable != null ) {
            log.println("Creating instance...");
            tEnv = new TestEnvironment(oObj);
        }

        log.println( "adding TextDocument as mod relation to environment" );
        tEnv.addObjRelation( "TEXTDOC", xTextDoc );
        tEnv.addObjRelation( "ROW", new Integer( nRow ) );
        tEnv.addObjRelation( "COL", new Integer( nCol ) );
        try {
            tEnv.addObjRelation( "INST", SOfficeFactory.createTextTable( xTextDoc ));
        }
        catch( Exception uE ) {
            uE.printStackTrace( log );
            throw new StatusException("Couldn't create TextTable : "
                    + uE.getMessage(), uE);
        }

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXTextTables

