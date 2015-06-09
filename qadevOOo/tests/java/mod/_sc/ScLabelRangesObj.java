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
import com.sun.star.sheet.XLabelRanges;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.LabelRanges</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XLabelRanges</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
* </ul>
* @see com.sun.star.sheet.LabelRanges
* @see com.sun.star.sheet.XLabelRanges
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see ifc.sheet._XLabelRanges
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
*/
public class ScLabelRangesObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());
        log.println( "creating a sheetdocument" );
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
    * Obtains the value of the property <code>'ColumnLabelRanges'</code>
    * from the document. The property value is the collection of label ranges.
    * Adds new label range to the collection using the interface
    * <code>XLabelRanges</code> that was queried from the property value.
    * This collection is the instance of the service
    * <code>com.sun.star.sheet.LabelRanges</code>.
    * @see com.sun.star.sheet.LabelRanges
    * @see com.sun.star.sheet.XLabelRanges
    */
    @Override
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        log.println("Getting test object ") ;
        XPropertySet docProps = UnoRuntime.queryInterface(XPropertySet.class, xSheetDoc);
        Object ranges = docProps.getPropertyValue("ColumnLabelRanges");
        XLabelRanges lRanges = UnoRuntime.queryInterface(XLabelRanges.class, ranges);

        log.println("Adding at least one element for ElementAccess interface");
        CellRangeAddress aRange2 = new CellRangeAddress((short)0, 0, 1, 0, 6);
        CellRangeAddress aRange1 = new CellRangeAddress((short)0, 0, 0, 0, 1);
        lRanges.addNew(aRange1, aRange2);

        oObj = lRanges;

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("testing...");

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ScLabelRangesObj

