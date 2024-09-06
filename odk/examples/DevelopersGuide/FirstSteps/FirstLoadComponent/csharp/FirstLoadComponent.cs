/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;

using com.sun.star.beans;
using com.sun.star.container;
using com.sun.star.frame;
using com.sun.star.lang;
using com.sun.star.sheet;
using com.sun.star.table;
using com.sun.star.uno;

try
{
    XComponentContext xContext = NativeBootstrap.bootstrap();
    if (xContext is null)
        Console.Error.WriteLine("Could not bootstrap office");

    XMultiComponentFactory xServiceManager = xContext.getServiceManager();

    IQueryInterface desktop = xServiceManager
        .createInstanceWithContext("com.sun.star.frame.Desktop", xContext);
    XComponentLoader xComponentLoader = desktop.query<XComponentLoader>();

    PropertyValue[] loadProps = Array.Empty<PropertyValue>();
    XComponent xSpreadsheetComponent = xComponentLoader
        .loadComponentFromURL("private:factory/scalc", "_blank", 0, loadProps);

    XSpreadsheetDocument xSpreadsheetDocument = xSpreadsheetComponent
        .query<XSpreadsheetDocument>();

    XSpreadsheets xSpreadsheets = xSpreadsheetDocument.getSheets();
    xSpreadsheets.insertNewByName("MySheet", 0);
    Type elemType = xSpreadsheets.getElementType();
    Console.WriteLine(elemType.FullName);

    Any sheet = xSpreadsheets.getByName("MySheet");
    XSpreadsheet xSpreadsheet = sheet.castOrDefault<XSpreadsheet>();

    XCell xCell = xSpreadsheet.getCellByPosition(0, 0);
    xCell.setValue(21);
    xCell = xSpreadsheet.getCellByPosition(0, 1);
    xCell.setValue(21);
    xCell = xSpreadsheet.getCellByPosition(0, 2);
    xCell.setFormula("=sum(A1:A2)");

    XPropertySet xCellProps = xCell.query<XPropertySet>();
    xCellProps.setPropertyValue("CellStyle", new Any("Result"));

    XModel xSpreadsheetModel = xSpreadsheetComponent.query<XModel>();
    XController xSpreadsheetController = xSpreadsheetModel.getCurrentController();
    XSpreadsheetView xSpreadsheetView = xSpreadsheetController.query<XSpreadsheetView>();
    xSpreadsheetView.setActiveSheet(xSpreadsheet);

    // Example usage of enum values
    xCellProps.setPropertyValue("VertJustify", new Any(CellVertJustify.TOP));

    // Example usage of PropertyValue structs
    loadProps = new PropertyValue[1]
    {
        new PropertyValue()
        {
            Name = "AsTemplate",
            Value = new Any(true),
        }
    };

    // Uncomment to load a Calc file as template
    //xSpreadsheetComponent = xComponentLoader.loadComponentFromURL(
    //    "file:///c:/temp/DataAnalysys.ods", "_blank", 0, loadProps);

    // Example usage of XEnumerationAccess
    XCellRangesQuery xCellQuery = sheet.castOrDefault<XCellRangesQuery>();
    XSheetCellRanges xFormulaCells = xCellQuery.queryContentCells(CellFlags.FORMULA);
    XEnumerationAccess xFormulas = xFormulaCells.getCells();
    XEnumeration xFormulaEnum = xFormulas.createEnumeration();

    while (xFormulaEnum.hasMoreElements())
    {
        Any formulaCell = xFormulaEnum.nextElement();
        xCell = formulaCell.castOrDefault<XCell>();
        XCellAddressable xCellAddress = xCell.query<XCellAddressable>();
        Console.WriteLine($"Formula cell in column {xCellAddress.getCellAddress().Column}, row {xCellAddress.getCellAddress().Row} contains {xCell.getFormula()}");
    }

    return 0;
}
catch (UnoException e)
{
    Console.Error.WriteLine(e.Message);

    return 1;
}