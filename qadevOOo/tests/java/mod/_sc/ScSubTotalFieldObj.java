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

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.GeneralFunction;
import com.sun.star.sheet.SubTotalColumn;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSubTotalCalculatable;
import com.sun.star.sheet.XSubTotalDescriptor;
import com.sun.star.sheet.XSubTotalField;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.SubTotalField</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XSubTotalField</code></li>
* </ul>
* @see com.sun.star.sheet.SubTotalField
* @see com.sun.star.sheet.XSubTotalField
* @see ifc.sheet._XSubTotalField
*/
public class ScSubTotalFieldObj extends TestCase {
    private XSpreadsheetDocument xSpreadsheetDoc;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    public void initialize( TestParameters Param, PrintWriter log ) throws Exception {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF());

        log.println("creating a spreadsheetdocument");
        xSpreadsheetDoc = SOF.createCalcDoc(null);
    }

    /**
    * Disposes Spreadsheet document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from the document and takes one of
    * them. Creates a subtotal descriptor using the interface
    * <code>XSubTotalCalculatable</code>. Adds a subtotal field definition to
    * the descriptor. Obtains the subtotal field with index 0 from the
    * collection. The obtained subtotal field is the instance of the service
    * <code>com.sun.star.sheet.SubTotalField</code>.
    * @see com.sun.star.sheet.XSubTotalCalculatable
    * @see com.sun.star.sheet.SubTotalField
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = xSpreadsheetDoc.getSheets();

        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        oSheet = (XSpreadsheet) AnyConverter.toObject(
                new Type (XSpreadsheet.class),oIndexAccess.getByIndex(0));

        XSubTotalCalculatable xSTC = UnoRuntime.queryInterface(XSubTotalCalculatable.class, oSheet);

        XSubTotalDescriptor xSTD = xSTC.createSubTotalDescriptor(true);

        SubTotalColumn[] columns = new SubTotalColumn[1];
        SubTotalColumn column = new SubTotalColumn();
        column.Column = 5;
        column.Function = GeneralFunction.SUM;
        columns[0] = column;
        xSTD.addNew(columns, 1);

        XIndexAccess oDescIndex = UnoRuntime.queryInterface(XIndexAccess.class, xSTD);

        XInterface oObj = null;

        oObj = ( XSubTotalField ) AnyConverter.toObject(
                new Type(XSubTotalField.class),oDescIndex.getByIndex(0));

        TestEnvironment tEnv = new TestEnvironment(oObj);
        return tEnv;

    } // finish method getTestEnvironment

}    // finish class ScSubTotalFieldObj

