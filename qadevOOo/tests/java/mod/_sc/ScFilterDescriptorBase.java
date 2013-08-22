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
import util.SOfficeFactory;

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSheetFilterable;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.SheetFilterDescriptor</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XSheetFilterDescriptor</code></li>
*  <li> <code>com::sun::star::sheet::SheetFilterDescriptor</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* @see com.sun.star.sheet.SheetFilterDescriptor
* @see com.sun.star.sheet.XSheetFilterDescriptor
* @see com.sun.star.sheet.SheetFilterDescriptor
* @see com.sun.star.beans.XPropertySet
* @see ifc.sheet._XSheetFilterDescriptor
* @see ifc.sheet._SheetFilterDescriptor
* @see ifc.beans._XPropertySet
*/
public class ScFilterDescriptorBase extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a sheetdocument" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
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
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Fills some cells in the spreadsheet.
    * Creates the filter descriptor using the interface
    * <code>XSheetFilterable</code>. This filter descriptor is the instance
    * of the service <code>com.sun.star.sheet.SheetFilterDescriptor</code>.
    * @see com.sun.star.sheet.XSheetFilterable
    * @see com.sun.star.sheet.SheetFilterDescriptor
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        XSheetFilterable SF = null;

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets();

        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);

        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexAccess.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        }

        log.println("filling some cells");
        try {
            oSheet.getCellByPosition(5, 5).setValue(15);
            oSheet.getCellByPosition(1, 4).setValue(10);
            oSheet.getCellByPosition(2, 0).setValue(-5.15);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception occurred while filling cells", e);
        }

        SF = UnoRuntime.queryInterface(XSheetFilterable.class, oSheet);

        oObj = SF.createFilterDescriptor(true);

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class ScFilterDescriptorBase

