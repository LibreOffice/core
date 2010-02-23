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
    static XSpreadsheetDocument xSheetDoc = null;


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
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSheetDoc);
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
            XIndexAccess oIndexSheets = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

            log.println("Getting a cell from sheet") ;
            oObj = oSheet.getCellByPosition(2, 3) ;
            cell = (XCell)UnoRuntime.queryInterface(XCell.class, oObj);

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

        XPropertySet PropSet = (XPropertySet)
                    UnoRuntime.queryInterface(XPropertySet.class, oObj);
        tEnv.addObjRelation("PropSet",PropSet);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ScCellObj

