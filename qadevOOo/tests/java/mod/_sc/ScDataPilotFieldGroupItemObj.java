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
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.DataPilotFieldGroupInfo;
import com.sun.star.sheet.XDataPilotDescriptor;
import com.sun.star.sheet.XDataPilotFieldGrouping;
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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;



public class ScDataPilotFieldGroupItemObj extends TestCase
{
    private XSpreadsheetDocument xSheetDoc = null;

    /**
     * A field is filled some values. This integer determines the size of the
     * field in x and y direction.
     */
    private static final int mMaxFieldIndex = 6;

    /**
     * Creates Spreadsheet document.
     */
    @Override
    protected void initialize (TestParameters tParam, PrintWriter log) throws Exception
    {
        SOfficeFactory SOF = SOfficeFactory.getFactory (
            tParam.getMSF ());

        log.println ("creating a Spreadsheet document");
        xSheetDoc = SOF.createCalcDoc (null);
    }

    /**
     * Disposes Spreadsheet document.
     */
    @Override
    protected void cleanup (TestParameters tParam, PrintWriter log)
    {
        log.println ("    disposing xSheetDoc ");

        XComponent oComp = UnoRuntime.queryInterface (
            XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc (oComp);
    }

    @Override
    protected TestEnvironment createTestEnvironment (TestParameters Param,
        PrintWriter log) throws Exception
    {
        XInterface oObj = null;
        XInterface datapilotfield = null;
        XInterface groups = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println ("Creating a test environment");

        // the cell range
        CellRangeAddress sCellRangeAdress = new CellRangeAddress ();
        sCellRangeAdress.Sheet = 0;
        sCellRangeAdress.StartColumn = 1;
        sCellRangeAdress.StartRow = 0;
        sCellRangeAdress.EndColumn = mMaxFieldIndex - 1;
        sCellRangeAdress.EndRow = mMaxFieldIndex - 1;

        // position of the data pilot table
        CellAddress sCellAdress = new CellAddress ();
        sCellAdress.Sheet = 0;
        sCellAdress.Column = 7;
        sCellAdress.Row = 8;

        log.println ("Getting a sheet");

        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets ();
        XSpreadsheet oSheet = null;
        XSpreadsheet oSheet2 = null;
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface (
            XIndexAccess.class, xSpreadsheets);
        // Make sure there are at least two sheets
        xSpreadsheets.insertNewByName("Some Sheet", (short)0);

        oSheet = (XSpreadsheet) AnyConverter.toObject (
            new Type (XSpreadsheet.class),
            oIndexAccess.getByIndex (0));
        oSheet2 = (XSpreadsheet) AnyConverter.toObject (
            new Type (XSpreadsheet.class),
            oIndexAccess.getByIndex (1));

        log.println ("Filling a table");

        for (int i = 1; i < mMaxFieldIndex; i++)
        {
            oSheet.getCellByPosition (i, 0).setFormula ("Col" + i);
            oSheet.getCellByPosition (0, i).setFormula ("Row" + i);
            oSheet2.getCellByPosition (i, 0).setFormula ("Col" + i);
            oSheet2.getCellByPosition (0, i).setFormula ("Row" + i);
        }

        for (int i = 1; i < mMaxFieldIndex; i++)
        {
            for (int j = 1; j < mMaxFieldIndex; j++)
            {
                oSheet.getCellByPosition (i, j).setValue (i * (j + 1));
                oSheet2.getCellByPosition (i, j).setValue (i * (j + 2));
            }
        }

        oSheet.getCellByPosition (1, 1).setFormula ("aName");
        oSheet.getCellByPosition (1, 2).setFormula ("otherName");
        oSheet.getCellByPosition (1, 3).setFormula ("una");
        oSheet.getCellByPosition (1, 4).setFormula ("otherName");
        oSheet.getCellByPosition (1, 5).setFormula ("somethingelse");

        oSheet.getCellByPosition (1, 5);

        int x = sCellAdress.Column;
        int y = sCellAdress.Row + 3;

        oSheet.getCellByPosition (x, y);


        // create the test objects
        log.println ("Getting test objects");

        XDataPilotTablesSupplier DPTS = UnoRuntime.queryInterface (
            XDataPilotTablesSupplier.class,
            oSheet);
        XDataPilotTables DPT = DPTS.getDataPilotTables ();
        XDataPilotDescriptor DPDsc = DPT.createDataPilotDescriptor ();
        DPDsc.setSourceRange (sCellRangeAdress);

        XPropertySet fieldPropSet = null;

        Object oDataPilotField = DPDsc.getDataPilotFields ().getByIndex (0);
        fieldPropSet = UnoRuntime.queryInterface (
            XPropertySet.class, oDataPilotField);
        fieldPropSet.setPropertyValue ("Orientation",
            com.sun.star.sheet.DataPilotFieldOrientation.ROW);
        oDataPilotField = DPDsc.getDataPilotFields ().getByIndex (1);
        fieldPropSet = UnoRuntime.queryInterface (
            XPropertySet.class, oDataPilotField);
        fieldPropSet.setPropertyValue ("Function",
            com.sun.star.sheet.GeneralFunction.SUM);
        fieldPropSet.setPropertyValue ("Orientation",
            com.sun.star.sheet.DataPilotFieldOrientation.DATA);
        oDataPilotField = DPDsc.getDataPilotFields ().getByIndex (2);
        fieldPropSet = UnoRuntime.queryInterface (
            XPropertySet.class, oDataPilotField);
        fieldPropSet.setPropertyValue ("Orientation",
            com.sun.star.sheet.DataPilotFieldOrientation.COLUMN);

        log.println ("Insert the DataPilotTable");

        if (DPT.hasByName ("DataPilotTable"))
        {
            DPT.removeByName ("DataPilotTable");
        }

        DPT.insertNewByName ("DataPilotTable", sCellAdress, DPDsc);
        XIndexAccess xIA = UnoRuntime.queryInterface (XIndexAccess.class,DPTS.getDataPilotTables ());
        XIndexAccess IA = null;
        XDataPilotDescriptor xDPT = UnoRuntime.queryInterface (XDataPilotDescriptor.class,xIA.getByIndex (0));
        IA = xDPT.getRowFields ();
        System.out.println ("COUNT: "+IA.getCount ());
        datapilotfield = (XInterface) AnyConverter.toObject (
            new Type (XInterface.class), IA.getByIndex (0));

        try
        {
            XDataPilotFieldGrouping  dpfg = UnoRuntime.queryInterface (XDataPilotFieldGrouping.class, datapilotfield);
            String[] elements = new String[]{"aName","otherName"};
            dpfg.createNameGroup (elements);
            DataPilotFieldGroupInfo dpgi=null;
            xIA = UnoRuntime.queryInterface (XIndexAccess.class,DPTS.getDataPilotTables ());
            XDataPilotDescriptor xDPT2 = UnoRuntime.queryInterface (XDataPilotDescriptor.class,xIA.getByIndex (0));
            IA = xDPT2.getRowFields ();
            for (int i=0;i<IA.getCount ();i++)
            {
                datapilotfield = (XInterface) AnyConverter.toObject (
                    new Type (XInterface.class), IA.getByIndex (i));
                XPropertySet xPropertySet = UnoRuntime.queryInterface (XPropertySet.class, IA.getByIndex (i));
                if (((Boolean)xPropertySet.getPropertyValue ("IsGroupField")).booleanValue ())
                {
                    xPropertySet = UnoRuntime.queryInterface (XPropertySet.class, datapilotfield);
                    XNamed xNamed = UnoRuntime.queryInterface (XNamed.class, IA.getByIndex (i));
                    System.out.println ("name: "+xNamed.getName ());
                    dpgi = (DataPilotFieldGroupInfo) xPropertySet.getPropertyValue ("GroupInfo");
                }
            }
            groups = dpgi.Groups;
            XIndexAccess groupAccess = UnoRuntime.queryInterface(XIndexAccess.class, groups);
            XNameAccess groupNames = UnoRuntime.queryInterface(XNameAccess.class, groupAccess.getByIndex(0));
            oObj = UnoRuntime.queryInterface(XInterface.class, groupNames.getByName("aName"));
        }
        catch (Exception e)
        {
            e.printStackTrace ();
        }

        log.println ("Creating object - " +
            ((oObj == null) ? "FAILED" : "OK"));

        TestEnvironment tEnv = new TestEnvironment (oObj);

        log.println ("Implementationname: " + util.utils.getImplName (oObj));

        // Other parameters required for interface tests
        return tEnv;
    }

}
