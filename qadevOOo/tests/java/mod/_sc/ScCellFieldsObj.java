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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object that represents a collection of text fields
 * in a cell of a spreadsheet. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
 *  <li> <code>com::sun::star::util::XRefreshable</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.container.XEnumerationAccess
 * @see com.sun.star.util.XRefreshable
 * @see com.sun.star.container.XElementAccess
 * @see ifc.container._XEnumerationAccess
 * @see ifc.util._XRefreshable
 * @see ifc.container._XElementAccess
 */
public class ScCellFieldsObj extends TestCase {
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
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.text.TextField.URL</code>, inserts it to the content
    * of the cell in the spreadsheet. Then the component is obtained
    * by <code>XTextFieldsSupplier</code> interface  of a cell.<p>
    */
    @Override
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XText oText = null;
        XTextContent oContent = null;
        XInterface aField = null;

        // we want to create an instance of ScCellFieldObj.
        // to do this we must get an MultiServiceFactory.

        XMultiServiceFactory _oMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xSheetDoc);

        aField = (XInterface)
            _oMSF.createInstance("com.sun.star.text.TextField.URL");
        oContent = UnoRuntime.queryInterface(XTextContent.class, aField);

        XSpreadsheets oSheets = xSheetDoc.getSheets() ;
        XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
        XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

        XCell oCell = oSheet.getCellByPosition(2,3);
        oText = UnoRuntime.queryInterface(XText.class, oCell);

        oText.insertTextContent(
            oText.createTextCursor(), oContent, true);

        XTextFieldsSupplier xTextFieldsSupp = UnoRuntime.queryInterface(XTextFieldsSupplier.class, oCell);

        oObj = xTextFieldsSupp.getTextFields();

        TestEnvironment tEnv = new TestEnvironment(oObj) ;

        return tEnv;
    }

}

