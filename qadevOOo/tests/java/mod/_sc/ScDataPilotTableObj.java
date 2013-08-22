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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.TableFilterField;
import com.sun.star.sheet.XDataPilotDescriptor;
import com.sun.star.sheet.XDataPilotTables;
import com.sun.star.sheet.XDataPilotTablesSupplier;
import com.sun.star.sheet.XSheetFilterDescriptor;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.DataPilotTable</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::sheet::XDataPilotTable</code></li>
*  <li> <code>com::sun::star::sheet::XDataPilotDescriptor</code></li>
* </ul>
* @see com.sun.star.sheet.DataPilotTable
* @see com.sun.star.container.XNamed
* @see com.sun.star.sheet.XDataPilotTable
* @see com.sun.star.sheet.XDataPilotDescriptor
* @see ifc.container._XNamed
* @see ifc.sheet._XDataPilotTable
* @see ifc.sheet._XDataPilotDescriptor
*/
public class ScDataPilotTableObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
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
        XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Fills some table in the spreadsheet.
    * Obtains the collection of data pilot tables using the interface
    * <code>XDataPilotTablesSupplier</code>. Creates a data pilot descriptor
    * for the filled table. Obtains the collection of all the data pilot fields
    * using the interface <code>XDataPilotDescriptor</code>. Sets field
    * orientation for every of the data pilot fields. Sets the definitions of
    * the filter fields for the created description. Inserts new data pilot
    * table with this descriptor to the collection.
    * This new data pilot table is the instance of the service
    * <code>com.sun.star.sheet.DataPilotTable</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'OUTPUTRANGE'</code> for
    *      {@link ifc.sheet._XDataPilotTable}(the cell range address of the
    *      created data pilot table) </li>
    * <li> <code>'FIELDSAMOUNT'</code> for
    *      {@link ifc.sheet._XDataPilotDescriptor}(the number of filled fields
    *      which descriptor was created for) </li>
    *  <li> <code>'CELLFORCHANGE'</code> for
    *      {@link ifc.sheet._XDataPilotTable}(value of this cell will be changed)</li>
    *  <li> <code>'CELLFORCHECK'</code> for
    *      {@link ifc.sheet._XDataPilotTable}(value of this cell must be changed
    *      after refresh call)</li>
    * </ul>
    * @see com.sun.star.sheet.DataPilotTable
    * @see com.sun.star.sheet.XDataPilotTablesSupplier
    * @see com.sun.star.sheet.XDataPilotDescriptor
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        CellAddress sCellAddress = new CellAddress();
        sCellAddress.Sheet = 0;
        sCellAddress.Column = 7;
        sCellAddress.Row = 8;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );
        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets();
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        XSpreadsheet oSheet = null;
        Object oChangeCell = null;
        Object oCheckCell = null;
        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexAccess.getByIndex(0));
            oChangeCell = oSheet.getCellByPosition(1, 5);
            oCheckCell = oSheet.getCellByPosition(
                sCellAddress.Column, sCellAddress.Row + 3);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        }
        log.println("Getting sheet - " + (oSheet == null ? "FAILED" : "OK"));
        try {
            log.println("Filing a table");
            for (int i = 1; i < 6; i++) {
                oSheet.getCellByPosition(0, i).setFormula("Row" + i);
                oSheet.getCellByPosition(i, 0).setFormula("Col" + i);
            }

            for (int i = 1; i < 6; i++)
                for (int j = 1; j < 6; j++) {
                    oSheet.getCellByPosition(i, j).setValue(2.5 * j + i);
                }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't fill some cells", e);
        }

        CellRangeAddress sCellRangeAddress = new CellRangeAddress();
        sCellRangeAddress.Sheet = 0;
        sCellRangeAddress.StartColumn = 1;
        sCellRangeAddress.StartRow = 0;
        sCellRangeAddress.EndColumn = 5;
        sCellRangeAddress.EndRow = 5;

        TableFilterField[] filterFields = new TableFilterField[2];
        filterFields[0] = new TableFilterField();
        filterFields[0].Connection = com.sun.star.sheet.FilterConnection.AND;
        filterFields[0].Field = 1;
        filterFields[0].IsNumeric = true;
        filterFields[0].NumericValue = 4;
        filterFields[0].Operator = com.sun.star.sheet.FilterOperator.GREATER;
        filterFields[1] = new TableFilterField();
        filterFields[1].Connection = com.sun.star.sheet.FilterConnection.AND;
        filterFields[1].Field = 1;
        filterFields[1].IsNumeric = true;
        filterFields[1].NumericValue = 12;
        filterFields[1].Operator = com.sun.star.sheet.FilterOperator.LESS_EQUAL;

        XDataPilotTablesSupplier DPTS = UnoRuntime.queryInterface(XDataPilotTablesSupplier.class, oSheet);
        log.println("Getting test object");
        XDataPilotTables DPT = DPTS.getDataPilotTables();
        XDataPilotDescriptor DPDsc = DPT.createDataPilotDescriptor();
        DPDsc.setSourceRange(sCellRangeAddress);

        XSheetFilterDescriptor SFD = DPDsc.getFilterDescriptor();
        SFD.setFilterFields(filterFields);

        XPropertySet fieldPropSet = null;
        try {
            Object oDataPilotField = DPDsc.getDataPilotFields().getByIndex(0);
            fieldPropSet = UnoRuntime.queryInterface(XPropertySet.class, oDataPilotField);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        }

        try {
            fieldPropSet.setPropertyValue("Function",
                com.sun.star.sheet.GeneralFunction.SUM);
            fieldPropSet.setPropertyValue("Orientation",
                com.sun.star.sheet.DataPilotFieldOrientation.DATA);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        }

        if (DPT.hasByName("DataPilotTable")) {
            DPT.removeByName("DataPilotTable");
        }
        DPT.insertNewByName("DataPilotTable", sCellAddress, DPDsc);
        try {
            oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),DPT.getByName(DPT.getElementNames()[0]));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        }

        log.println("Creating object - " +
                                    ((oObj == null) ? "FAILED" : "OK"));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Other parameters required for interface tests
        tEnv.addObjRelation("OUTPUTRANGE", sCellAddress);
        tEnv.addObjRelation("CELLFORCHANGE", oChangeCell);
        tEnv.addObjRelation("CELLFORCHECK", oCheckCell);
        tEnv.addObjRelation("FIELDSAMOUNT", new Integer(5));
        tEnv.addObjRelation("SHEETDOCUMENT", xSheetDoc);

        createTable2(oSheet, sCellRangeAddress, tEnv);

        return tEnv;
    }

    /**
     * Create a new DataPilot table output for use with testing XDataPilotTable2
     * interface.
     *
     * @param oSheet current sheet instance
     * @param srcRange source range
     * @param tEnv test environment instance
     */
    private void createTable2(XSpreadsheet oSheet, CellRangeAddress srcRange, TestEnvironment tEnv)
    {
        XDataPilotTablesSupplier DPTS = UnoRuntime.queryInterface(XDataPilotTablesSupplier.class, oSheet);
        log.println("Creating test table object");
        XDataPilotTables DPT = DPTS.getDataPilotTables();
        XDataPilotDescriptor DPDsc = DPT.createDataPilotDescriptor();
        DPDsc.setSourceRange(srcRange);

        XIndexAccess xIA = DPDsc.getDataPilotFields();
        int fieldCount = xIA.getCount() - 1; // skip the last field because it's always hidden.
        try
        {
            for (int i = 0; i < fieldCount; ++i)
            {
                Object o = xIA.getByIndex(i);
                XPropertySet fieldPropSet = UnoRuntime.queryInterface(
                    XPropertySet.class, o);

                if (i == fieldCount - 1)
                {
                    // last field
                    fieldPropSet.setPropertyValue(
                        "Function", com.sun.star.sheet.GeneralFunction.SUM);
                    fieldPropSet.setPropertyValue(
                        "Orientation", com.sun.star.sheet.DataPilotFieldOrientation.DATA);
                }
                else if (i%2 == 0)
                {
                    // even number fields
                    fieldPropSet.setPropertyValue(
                        "Orientation", com.sun.star.sheet.DataPilotFieldOrientation.COLUMN);
                }
                else if (i%2 == 1)
                {
                    // odd number fields
                    fieldPropSet.setPropertyValue(
                        "Orientation", com.sun.star.sheet.DataPilotFieldOrientation.ROW);
                }
            }

            if (DPT.hasByName("DataPilotTable2"))
                DPT.removeByName("DataPilotTable2");

            CellAddress destAddr = new CellAddress();
            destAddr.Sheet = 0;
            destAddr.Column = 0;
            destAddr.Row = 14;
            DPT.insertNewByName("DataPilotTable2", destAddr, DPDsc);

            Object o = DPT.getByName("DataPilotTable2");
            tEnv.addObjRelation("DATAPILOTTABLE2", o);
        }
        catch (com.sun.star.uno.Exception e)
        {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        }
    }

}


