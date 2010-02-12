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
import util.SOfficeFactory;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.Spreadsheets</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XSpreadsheets</code></li>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XNameReplace</code></li>
*  <li> <code>com::sun::star::container::XNameContainer</code></li>
* </ul>
* @see com.sun.star.sheet.Spreadsheets
* @see com.sun.star.sheet.XSpreadsheets
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XNameReplace
* @see com.sun.star.container.XNameContainer
* @see ifc.sheet._XSpreadsheets
* @see ifc.container._XNameAccess
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XNameReplace
* @see ifc.container._XNameContainer
*/
public class ScTableSheetsObj extends TestCase {
    private static XSpreadsheetDocument xSpreadsheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    public void initialize( TestParameters Param, PrintWriter log ) {
        // get a soffice factory object

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());
        try {
            log.println("creating a spreadsheetdocument");
            xSpreadsheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document ", e );
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println("disposing xSpreadsheetDocument");
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of the spreadsheets using the interface
    * <code>XSpreadsheetDocument</code>. The retrieved collection is the
    * instance of the service <code>com.sun.star.sheet.Spreadsheets</code>.
    * Creates instances of the service <code>com.sun.star.sheet.Spreadsheet</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'SecondInstance'</code> for
    *      {@link ifc.container._XNameContainer} (the created instance of the
    *       service <code>com.sun.star.sheet.Spreadsheet</code>) </li>
    *  <li> <code>'INSTANCE1', ..., 'INSTANCEN'</code> for
    *      {@link ifc.container._XNameContainer} (the created instances of the
    *       service <code>com.sun.star.sheet.Spreadsheet</code>) </li>
    * </ul>
    * @see com.sun.star.sheet.Spreadsheets
    * @see com.sun.star.sheet.XSpreadsheetDocument
    * @see com.sun.star.sheet.Spreadsheet
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = (XSpreadsheets)xSpreadsheetDoc.getSheets();

        XInterface oObj = (XInterface)
            UnoRuntime.queryInterface(XInterface.class, xSpreadsheets);

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("creating instance of the sheet object to use it in tests");
        XSpreadsheet oSecondSheet = null;

        try {
            oSecondSheet = SOF.createSpreadsheet(xSpreadsheetDoc);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instances", e);
        }

        //adding Instance for XNameContainer
        tEnv.addObjRelation("SecondInstance",oSecondSheet);

        // INSTANCEn : _XNameContainer; _XNameReplace
        log.println( "adding INSTANCEn as mod relation to environment" );

        int THRCNT = 1;
        if ((String)Param.get("THRCNT") != null) {
            THRCNT = Integer.parseInt((String)Param.get("THRCNT"));
        }
        try {
            for (int n = 1; n < (THRCNT+1) ;n++ ) {
                log.println(
                    "adding INSTANCE" + n +" as mod relation to environment" );
                tEnv.addObjRelation(
                    "INSTANCE" + n,
                    SOF.createSpreadsheet(xSpreadsheetDoc) );
                }
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Couldn't create instances for object relations", e);
        }

        return tEnv;
    }
}


