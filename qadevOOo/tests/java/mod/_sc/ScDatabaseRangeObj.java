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

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XCellRangeReferrer;
import com.sun.star.sheet.XDatabaseRanges;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XImportable;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;


/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.DatabaseRange</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::sheet::DatabaseRange</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::sheet::XDatabaseRange</code></li>
*  <li> <code>com::sun::star::sheet::XCellRangeReferrer</code></li>
* </ul>
* @see com.sun.star.sheet.DatabaseRange
* @see com.sun.star.container.XNamed
* @see com.sun.star.sheet.DatabaseRange
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.sheet.XDatabaseRange
* @see com.sun.star.sheet.XCellRangeReferrer
* @see ifc.container._XNamed
* @see ifc.sheet._DatabaseRange
* @see ifc.beans._XPropertySet
* @see ifc.sheet._XDatabaseRange
* @see ifc.sheet._XCellRangeReferrer
*/
public class ScDatabaseRangeObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a Spreadsheet document");
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xSheetDoc ");

        XComponent oComp = UnoRuntime.queryInterface(
                                   XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of database ranges in the document.
    * If the database range with name <code>'dbRange'</code> exists
    * in the collection then removes it from the collection.
    * Creates new database range and adds it to the collection with the name
    * <code>'dbRange'</code>.The database range that was added to the collection
    * is the instance of the service <code>com.sun.star.sheet.DatabaseRange</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'DATAAREA'</code> for
    *      {@link ifc.sheet._XCellRangeReferrer}(of type
    *      <code>CellRangeAddress</code>)</li>
    *  <li> <code>'XCELLRANGE'</code> (of type <code>XCellRange</code>):
    *   cell range of the spreadsheet with database range</li>
    * </ul>
    * @see com.sun.star.sheet.DatabaseRange
    * @see com.sun.star.table.CellRangeAddress
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("Creating a test environment");

        log.println("Getting test object ");

        XPropertySet docProps = UnoRuntime.queryInterface(
                                        XPropertySet.class, xSheetDoc);

        XSpreadsheets sheets = xSheetDoc.getSheets();
        String[] names = sheets.getElementNames();
        XDatabaseRanges dbRanges = null;
        XImportable xImp = null;

        try {
            Object sheet = sheets.getByName(names[0]);
            xImp = UnoRuntime.queryInterface(XImportable.class,
                                                           sheet);
            dbRanges = (XDatabaseRanges) AnyConverter.toObject(
                               new Type(XDatabaseRanges.class),
                               docProps.getPropertyValue("DatabaseRanges"));
            _doImport(xImp);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a property", e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a property", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a property", e);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException(
                    "Error getting test object from spreadsheet document", e);
        }

        String dbName = "dbRange";

        if (dbRanges.hasByName(dbName)) {
            dbRanges.removeByName(dbName);
        }


        //CellRangeAddress aRange = new CellRangeAddress((short)0, 0, 0, 0, 13);
        CellRangeAddress aRange = null;

        //dbRanges.addNewByName("dbRange", aRange);
        XNameAccess dbrNA = UnoRuntime.queryInterface(
                                    XNameAccess.class, dbRanges);
        try {
            // we need to add it
            dbRanges.addNewByName(dbName,new CellRangeAddress((short)0, 0, 0, 0, 5));

            UnoRuntime.queryInterface(XNamed.class,
                                                        dbrNA.getByName(
                                                                dbName));

            XCellRangeReferrer aReferrer = UnoRuntime.queryInterface(
                                                   XCellRangeReferrer.class,
                                                   dbrNA.getByName(dbName));
            XCellRangeAddressable aRangeA = UnoRuntime.queryInterface(
                                                    XCellRangeAddressable.class,
                                                    aReferrer.getReferredCells());
            aRange = aRangeA.getRangeAddress();
            oObj = (XInterface) AnyConverter.toObject(
                           new Type(XInterface.class),
                           dbrNA.getByName(dbName));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(
                    "Error getting test object from spreadsheet document", e);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException(
                    "Error getting test object from spreadsheet document", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException(
                    "Error getting test object from spreadsheet document", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);


        // Other parameters required for interface tests
        tEnv.addObjRelation("DATAAREA", aRange);

        XCellRange xCellRange = null;

        try {
            Object sheet = sheets.getByName(names[0]);
            xCellRange = UnoRuntime.queryInterface(
                                 XCellRange.class, sheet);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(
                    "Error getting of first spreadsheet from spreadsheet" +
                    " document", e);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException(
                    "Error getting of first spreadsheet from spreadsheet" +
                    " document", e);
        }

        tEnv.addObjRelation("XCELLRANGE", xCellRange);

        return tEnv;
    }

    public void _doImport(XImportable imp) {
        PropertyValue[] descriptor = imp.createImportDescriptor(false);

        log.print("Setting the ImportDescriptor (Bibliograpy, SQL, select Identifier from biblio) -- ");
        descriptor[0].Value = "Bibliography";
        descriptor[1].Value = com.sun.star.sheet.DataImportMode.SQL;
        descriptor[2].Value = "select Identifier from biblio";
        log.println("done");

        log.print("Importing data (Bibliograpy, Table, biblio) -- ");
        imp.doImport(descriptor);
        log.println("done");
    }
}
