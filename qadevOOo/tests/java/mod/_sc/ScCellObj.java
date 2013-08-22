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
package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DefaultDsc;
import util.InstCreator;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import ifc.sheet._XCellRangesQuery;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.SheetCell</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::table::CellProperties</code></li>
*  <li> <code>com::sun::star::text::XSimpleText</code></li>
*  <li> <code>com::sun::star::table::XCell</code></li>
*  <li> <code>com::sun::star::text::XTextRange</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::document::XActionLockable</code></li>
*  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
*  <li> <code>com::sun::star::text::XText</code></li>
*  <li> <code>com::sun::star::sheet::XCellAddressable</code></li>
* </ul>
* @see com.sun.star.sheet.SheetCell
* @see com.sun.star.table.CellProperties
* @see com.sun.star.text.XSimpleText
* @see com.sun.star.table.XCell
* @see com.sun.star.text.XTextRange
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.document.XActionLockable
* @see com.sun.star.style.ParagraphProperties
* @see com.sun.star.text.XText
* @see com.sun.star.sheet.XCellAddressable
* @see ifc.table._CellProperties
* @see ifc.text._XSimpleText
* @see ifc.table._XCell
* @see ifc.text._XTextRange
* @see ifc.beans._XPropertySet
* @see ifc.style._CharacterProperties
* @see ifc.document._XActionLockable
* @see ifc.style._ParagraphProperties
* @see ifc.text._XText
* @see ifc.sheet._XCellAddressable
*/
public class ScCellObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;


    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "Creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document,
    * and takes one of them. Retrieves some cell from the spreadsheet
    * that is instance of the service <code>com.sun.star.sheet.SheetCell</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'XTEXTINFO',</code> for
    *      {@link ifc.text._XText} </li>
    * </ul>
    * @see com.sun.star.sheet.SheetCell
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        XSpreadsheet oSheet = null;
        XCell cell = null;
        try {
            log.println("Getting spreadsheet") ;
            XSpreadsheets oSheets = xSheetDoc.getSheets() ;
            XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

            log.println("Getting a cell from sheet") ;
            oObj = oSheet.getCellByPosition(2, 3) ;
            cell = UnoRuntime.queryInterface(XCell.class, oObj);

        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Error getting cell object from spreadsheet document", e);
        }

        log.println( "creating a new environment for ScCellObj object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Object relations for interface tests
        DefaultDsc tDsc = new DefaultDsc(
            "com.sun.star.text.XTextContent", "com.sun.star.text.TextField.URL");
        log.println( "    adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xSheetDoc, tDsc ) );
        // add the sheet
        tEnv.addObjRelation("SHEET", oSheet);
        // add expected results for the XCellRangesQuery interface test
        String[]expectedResults = new String[7];

        expectedResults[_XCellRangesQuery.QUERYCOLUMNDIFFERENCES] = "Sheet1.C4";
        expectedResults[_XCellRangesQuery.QUERYCONTENTCELLS] = "";
        expectedResults[_XCellRangesQuery.QUERYEMPTYCELLS] = "Sheet1.C4";
        expectedResults[_XCellRangesQuery.QUERYFORMULACELLS] = "";
        expectedResults[_XCellRangesQuery.QUERYINTERSECTION] = "";
        expectedResults[_XCellRangesQuery.QUERYROWDIFFERENCES] = "Sheet1.C4";
        expectedResults[_XCellRangesQuery.QUERYVISIBLECELLS] = "Sheet1.C4";
        tEnv.addObjRelation("XCellRangesQuery.EXPECTEDRESULTS", expectedResults);
        tEnv.addObjRelation("XCellRangesQuery.CREATEENTRIES", Boolean.TRUE);

        // make entries in this cell at the interface test
        tEnv.addObjRelation("XTextFieldsSupplier.MAKEENTRY", Boolean.TRUE);
        tEnv.addObjRelation("MAKEENTRYINCELL", cell);

        // for XSearchable amd XReplaceable interface test
        tEnv.addObjRelation("XSearchable.MAKEENTRYINCELL", cell);
        tEnv.addObjRelation("EXCLUDEFINDNEXT", Boolean.TRUE);

        // for XFormulaQuery interface test
        tEnv.addObjRelation("EXPECTEDDEPENDENTVALUES", new int[]{2,2,3,3});
        tEnv.addObjRelation("EXPECTEDPRECEDENTVALUES", new int[]{0,3,0,0});
        tEnv.addObjRelation("RANGEINDICES", new int[]{0,0});

        // XTextFieldsSupplier
        tEnv.addObjRelation("SPREADSHEET", xSheetDoc);

        XPropertySet PropSet = UnoRuntime.queryInterface(XPropertySet.class, oObj);
        tEnv.addObjRelation("PropSet",PropSet);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ScCellObj

