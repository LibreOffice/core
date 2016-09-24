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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XNamedRanges;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.NamedRange</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::sheet::XNamedRange</code></li>
*  <li> <code>com::sun::star::sheet::XCellRangeReferrer</code></li>
* </ul>
* @see com.sun.star.sheet.NamedRange
* @see com.sun.star.container.XNamed
* @see com.sun.star.sheet.XNamedRange
* @see com.sun.star.sheet.XCellRangeReferrer
* @see ifc.container._XNamed
* @see ifc.sheet._XNamedRange
* @see ifc.sheet._XCellRangeReferrer
*/
public class ScNamedRangeObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );

        log.println( "creating a Spreadsheet document" );
        xSheetDoc = SOF.createCalcDoc(null);
    }

    /**
    * Disposes Spreadsheet document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Obtains the value of the property
    * <code>'NamedRanges'</code> that is the collection of named ranges.
    * Creates and adds new range to the collection. This new range is the instance of the
    * service <code>com.sun.star.sheet.NamedRange</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'DATAAREA'</code> for
    *      {@link ifc.sheet._XCellRangeReferrer} (the cell range address of the
    *      created range) </li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        log.println("Getting test object ") ;

        // Getting named ranges.
        XPropertySet docProps = UnoRuntime.queryInterface(XPropertySet.class, xSheetDoc);

        Object ranges = docProps.getPropertyValue("NamedRanges");

        XNamedRanges xNamedRanges = UnoRuntime.queryInterface(XNamedRanges.class, ranges);

        CellRangeAddress DataArea = new CellRangeAddress((short)0, 0, 0, 1, 1);
        CellAddress base = new CellAddress(DataArea.Sheet,
                                           DataArea.StartColumn,
                                           DataArea.StartRow);

        if (xNamedRanges.hasByName("ANamedRange")) {
            xNamedRanges.removeByName("ANamedRange");
        }

        xNamedRanges.addNewByName("ANamedRange", "A1:B2", base, 0);

        CellAddress listOutputPosition = new CellAddress((short)0, 1, 1);
        xNamedRanges.outputList(listOutputPosition);

        oObj = (XInterface) AnyConverter.toObject(
            new Type(XInterface.class),xNamedRanges.getByName("ANamedRange"));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Other parameters required for interface tests
        tEnv.addObjRelation("DATAAREA", DataArea);

        return tEnv;
    }

}


