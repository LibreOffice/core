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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.DataPilotFieldOrientation;
import com.sun.star.sheet.XDataPilotDescriptor;
import com.sun.star.sheet.XDataPilotField;
import com.sun.star.sheet.XDataPilotTables;
import com.sun.star.sheet.XDataPilotTablesSupplier;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;


/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.DataPilotField</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::sheet::DataPilotField</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* @see com.sun.star.sheet.DataPilotField
* @see com.sun.star.container.XNamed
* @see com.sun.star.sheet.DataPilotField
* @see com.sun.star.beans.XPropertySet
* @see ifc.container._XNamed
* @see ifc.sheet._DataPilotField
* @see ifc.beans._XPropertySet
*/
public class ScIndexEnumeration_DataPilotItemsEnumeration
    extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
     * A field is filled some values. This integer determines the size of the
     * field in x and y direction.
     */
    private int mMaxFieldIndex = 6;

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
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Fills some table in the spreadsheet.
    * Obtains the collection of data pilot tables using the interface
    * <code>XDataPilotTablesSupplier</code>. Creates a data pilot descriptor
    * for the filled table and inserts new data pilot table with this descriptor
    * to the collection. Obtains the collection of all the data pilot fields
    * using the interface <code>XDataPilotDescriptor</code>. Retrieves from
    * the collection the data pilot field with index 0. This data pilot field
    * is the instance of the service <code>com.sun.star.sheet.DataPilotField</code>.
    * @see com.sun.star.sheet.DataPilotField
    * @see com.sun.star.sheet.XDataPilotTablesSupplier
    * @see com.sun.star.sheet.XDataPilotDescriptor
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("Creating a test environment");

        // the cell range
        CellRangeAddress sCellRangeAdress = new CellRangeAddress();
        sCellRangeAdress.Sheet = 0;
        sCellRangeAdress.StartColumn = 1;
        sCellRangeAdress.StartRow = 0;
        sCellRangeAdress.EndColumn = mMaxFieldIndex - 1;
        sCellRangeAdress.EndRow = mMaxFieldIndex - 1;

        // position of the data pilot table
        CellAddress sCellAdress = new CellAddress();
        sCellAdress.Sheet = 0;
        sCellAdress.Column = 7;
        sCellAdress.Row = 8;

        log.println("Getting a sheet");

        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets();
        XSpreadsheet oSheet = null;
        XSpreadsheet oSheet2 = null;
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(
                                            XIndexAccess.class, xSpreadsheets);
        // Make sure there are at least two sheets
        xSpreadsheets.insertNewByName("Some Sheet", (short)0);

        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                             new Type(XSpreadsheet.class),
                             oIndexAccess.getByIndex(0));
            oSheet2 = (XSpreadsheet) AnyConverter.toObject(
                              new Type(XSpreadsheet.class),
                              oIndexAccess.getByIndex(1));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't get a spreadsheet", e);
        }

        try {
            log.println("Filling a table");

            for (int i = 1; i < mMaxFieldIndex; i++) {
                oSheet.getCellByPosition(i, 0).setFormula("Col" + i);
                oSheet.getCellByPosition(0, i).setFormula("Row" + i);
                oSheet2.getCellByPosition(i, 0).setFormula("Col" + i);
                oSheet2.getCellByPosition(0, i).setFormula("Row" + i);
            }

            for (int i = 1; i < mMaxFieldIndex; i++)
                for (int j = 1; j < mMaxFieldIndex; j++) {
                    oSheet.getCellByPosition(i, j).setValue(i * (j + 1));
                    oSheet2.getCellByPosition(i, j).setValue(i * (j + 2));
                }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't fill some cells", e);
        }

        try {
            oSheet.getCellByPosition(1, 5);

            int x = sCellAdress.Column;
            int y = sCellAdress.Row + 3;


            oSheet.getCellByPosition(x, y);
            new Integer(27);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't get cells for changing.", e);
        }


        // create the test objects
        log.println("Getting test objects");

        XDataPilotTablesSupplier DPTS = UnoRuntime.queryInterface(
                                                XDataPilotTablesSupplier.class,
                                                oSheet);
        XDataPilotTables DPT = DPTS.getDataPilotTables();
        XDataPilotDescriptor DPDsc = DPT.createDataPilotDescriptor();
        DPDsc.setSourceRange(sCellRangeAdress);

        XPropertySet fieldPropSet = null;

        try {
            Object oDataPilotField = DPDsc.getDataPilotFields().getByIndex(0);
            fieldPropSet = UnoRuntime.queryInterface(
                                   XPropertySet.class, oDataPilotField);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        }

        try {
            fieldPropSet.setPropertyValue("Function",
                                          com.sun.star.sheet.GeneralFunction.SUM);
            fieldPropSet.setPropertyValue("Orientation",
                                          com.sun.star.sheet.DataPilotFieldOrientation.DATA);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't create a test environment", e);
        }

        log.println("Insert the DataPilotTable");

        if (DPT.hasByName("DataPilotTable")) {
            DPT.removeByName("DataPilotTable");
        }

        XIndexAccess IA = DPDsc.getDataPilotFields();
        getSRange(IA);

        DPT.insertNewByName("DataPilotTable", sCellAdress, DPDsc);

        try {
            oObj = (XInterface) AnyConverter.toObject(
                           new Type(XInterface.class), IA.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't get data pilot field", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't get data pilot field", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace();
            throw new StatusException("Couldn't get data pilot field", e);
        }

        log.println("Creating object - " +
                    ((oObj == null) ? "FAILED" : "OK"));

        XDataPilotField xDataPilotField = UnoRuntime.queryInterface(
                                                  XDataPilotField.class, oObj);

        XEnumerationAccess xEnumerationAccess = UnoRuntime.queryInterface(
                                                        XEnumerationAccess.class,
                                                        xDataPilotField.getItems());

        oObj = xEnumerationAccess.createEnumeration();

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("Implementationname: " + util.utils.getImplName(oObj));

        // Other parameters required for interface tests
        return tEnv;
    }

    private void getSRange(XIndexAccess IA) {
        int fieldsAmount = IA.getCount() + 1;

        String[] fieldsNames = new String[fieldsAmount];

        int i = -1;
        int cnt = 0;

        while ((++i) < fieldsAmount) {
            Object field;

            try {
                field = IA.getByIndex(i);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace(log);

                return;
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                e.printStackTrace(log);

                return;
            }

            XNamed named = UnoRuntime.queryInterface(XNamed.class,
                                                              field);
            String name = named.getName();

            log.println("**Field : '" + name + "' ... ");

            if (!name.equals("Data")) {
                fieldsNames[cnt] = name;

                XPropertySet props = UnoRuntime.queryInterface(
                                             XPropertySet.class, field);

                try {
                    switch (cnt % 5) {
                    case 0:
                        props.setPropertyValue("Orientation",
                                               DataPilotFieldOrientation.COLUMN);
                        log.println("  Column");

                        break;

                    case 1:
                        props.setPropertyValue("Orientation",
                                               DataPilotFieldOrientation.ROW);
                        log.println("  Row");

                        break;

                    case 2:
                        props.setPropertyValue("Orientation",
                                               DataPilotFieldOrientation.DATA);
                        log.println("  Data");

                        break;

                    case 3:
                        props.setPropertyValue("Orientation",
                                               DataPilotFieldOrientation.HIDDEN);
                        log.println("  Hidden");

                        break;

                    case 4:
                        props.setPropertyValue("Orientation",
                                               DataPilotFieldOrientation.PAGE);
                        log.println("  Page");

                        break;
                    }
                } catch (com.sun.star.lang.WrappedTargetException e) {
                    e.printStackTrace(log);

                    return;
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    e.printStackTrace(log);

                    return;
                } catch (com.sun.star.beans.PropertyVetoException e) {
                    e.printStackTrace(log);

                    return;
                } catch (com.sun.star.beans.UnknownPropertyException e) {
                    e.printStackTrace(log);

                    return;
                }

                if ((++cnt) > 4) {
                    break;
                }
            } else {
                return;
            }
        }
    }
}
