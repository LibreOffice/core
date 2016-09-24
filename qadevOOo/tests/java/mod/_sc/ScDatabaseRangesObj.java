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
import com.sun.star.sheet.XDatabaseRanges;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.DatabaseRanges</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XDatabaseRanges</code></li>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
* </ul>
* @see com.sun.star.sheet.DatabaseRanges
* @see com.sun.star.sheet.XDatabaseRanges
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see ifc.sheet._XDatabaseRanges
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
*/
public class ScDatabaseRangesObj extends TestCase {
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
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves the collection of database ranges in the document.
    * If the database range with name <code>'dbRange'</code> doesn't exist
    * in the collection then creates new database range and adds it to the
    * collection with the name <code>'dbRange'</code> to have one element
    * for the test of the interface <code>ElementAccess</code> at least.
    * The collection of database ranges is the instance of the service
    * <code>com.sun.star.sheet.DatabaseRanges</code>.
    * @see com.sun.star.sheet.DatabaseRanges
    * @see com.sun.star.container.XElementAccess
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        log.println("Getting test object ") ;
        XPropertySet docProps = UnoRuntime.queryInterface(XPropertySet.class, xSheetDoc);

        XDatabaseRanges dbRanges = null;
        dbRanges = (XDatabaseRanges) AnyConverter.toObject(
            new Type(XDatabaseRanges.class),
                docProps.getPropertyValue("DatabaseRanges"));

        log.println("Adding at least one element for ElementAccess interface");
        CellRangeAddress aRange = new CellRangeAddress((short)0, 2, 4, 5, 6);
        if (!dbRanges.hasByName("dbRange")) {
            dbRanges.addNewByName("dbRange", aRange);
        }

        oObj = dbRanges;
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Other parameters required for interface tests
        return tEnv;
    }

}


