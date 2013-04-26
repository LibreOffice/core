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
import com.sun.star.table.XCellRange;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.table.CellRange</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::CellProperties</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.table.CellRange
 * @see com.sun.star.text.CellProperties
 * @see com.sun.star.beans.XPropertySet
 * @see ifc.text._CellProperties
 * @see ifc.beans._XPropertySet
 */
public class SwXCellRange extends TestCase {
    SOfficeFactory SOF;
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
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
    * Creating a Testenvironment for the interfaces to be tested. At first
    * method creates and initializes the table, then if text document has no
    * tables, previously created table is inserted to text document. Finally,
    * custom cell range is gotten from the table created.
    *     Object relations created :
    * <ul>
    *  <li> <code>'CellProperties.TextSection'</code> for
    *    {@link ifc.text._CellProperties} : range of complete paragraphs
    *  within a text</li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XTextTable the_table = null;
        XInterface oObj = null;
        try {
            the_table = SOfficeFactory.createTextTable( xTextDoc );
            the_table.initialize(5, 5);
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't create testobj: "
                +e.getMessage(),e);
        }

        if( SOfficeFactory.getTableCollection( xTextDoc ).getCount() == 0 ) {
            try {
                SOfficeFactory.insertTextContent(xTextDoc, the_table );
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                e.printStackTrace( log );
                throw new StatusException("Couldn't create TextTable : "
                        + e.getMessage(), e);
            }
        }
        try {
            XCellRange the_Range = UnoRuntime.queryInterface(XCellRange.class, the_table);
            oObj = the_Range.getCellRangeByPosition(0, 0, 3, 4);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get CellRange : "
                    + e.getMessage(), e);
        }

        log.println("Creating instance...");
        TestEnvironment tEnv = new TestEnvironment( oObj );
        log.println("ImplName: " + util.utils.getImplName(oObj));

        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
        try {
            XInterface oTS = (XInterface)
                oDocMSF.createInstance("com.sun.star.text.TextSection");
            log.println("  adding TextSection object");
            tEnv.addObjRelation("CellProperties.TextSection", oTS);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Could not get instance of TextSection");
            e.printStackTrace(log);
        }

        Object[][] NewData = new Object[5][];
        NewData[0] = new Double[]
            {new Double(2.5),new Double(5),new Double(2.5),new Double(5)};
        NewData[1] = new Double[]
            {new Double(4),new Double(9),new Double(2.5),new Double(5)};
        NewData[2] = new Double[]
            {new Double(2.5),new Double(5),new Double(2.5),new Double(5)};
        NewData[3] = new Double[]
            {new Double(2.5),new Double(5),new Double(2.5),new Double(5)};
        NewData[4] = new Double[]
            {new Double(4),new Double(9),new Double(2.5),new Double(5)};
        tEnv.addObjRelation("NewData",NewData);

        // com::sun::star::chart::XChartDataArray
        tEnv.addObjRelation("CRDESC",
                 "Column and RowDescriptions can't be changed for this Object");


        return tEnv;
    }
}    // finish class SwXCellRange

