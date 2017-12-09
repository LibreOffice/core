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
import util.utils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSheetLinkable;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.SheetLink</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::util::XRefreshable</code></li>
*  <li> <code>com::sun::star::sheet::SheetLink</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* The following files used by this test :
* <ul>
*  <li><b> ScSheetLinksObj.sdc </b> : for creating link </li>
* </ul> <p>
* @see com.sun.star.sheet.SheetLink
* @see com.sun.star.container.XNamed
* @see com.sun.star.util.XRefreshable
* @see com.sun.star.sheet.SheetLink
* @see com.sun.star.beans.XPropertySet
* @see ifc.container._XNamed
* @see ifc.util._XRefreshable
* @see ifc.sheet._SheetLink
* @see ifc.beans._XPropertySet
*/
public class ScSheetLinkObj extends TestCase {
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
        XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Links the sheet to another sheet in another
    * document using the interface <code>XSheetLinkable</code>. Obtains the
    * value of the property <code>'SheetLinks'</code> that is the collection of
    * sheet links. Retrieves from the collection one of the sheet link that
    * is the instance of the service <code>com.sun.star.sheet.SheetLink</code>.
    * @see com.sun.star.sheet.SheetLink
    * @see com.sun.star.sheet.XSheetLinkable
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );
        XSpreadsheet oSheet = null;

        log.println("Getting test object ") ;
        XSpreadsheets oSheets = xSheetDoc.getSheets() ;
        XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
        oSheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

        XSheetLinkable SL = UnoRuntime.queryInterface(XSheetLinkable.class, oSheet);

        // creating link. Doesn't matter that it refers to unexistent object.
        // this is for proper work of XAccess tests.
        String sURL = utils.getFullTestDocName("ScSheetLinksObj.ods");
        SL.link(sURL, "Sheet1", "", "", com.sun.star.sheet.SheetLinkMode.VALUE);

        // Getting links.
        XPropertySet docProps = UnoRuntime.queryInterface(XPropertySet.class, xSheetDoc);
        Object oLinks = docProps.getPropertyValue("SheetLinks");

        XNameAccess links = UnoRuntime.queryInterface(XNameAccess.class, oLinks);

        String[] names = links.getElementNames();

        oObj = (XInterface)AnyConverter.toObject(
                    new Type(XInterface.class),links.getByName(names[0]));

        log.println("Creating object - " +
            ((oObj == null) ? "FAILED" : "OK"));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    }

}
