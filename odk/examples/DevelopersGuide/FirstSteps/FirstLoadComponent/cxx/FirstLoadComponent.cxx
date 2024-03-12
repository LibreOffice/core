/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <sal/main.h>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>

using namespace rtl;
using namespace cppu;
using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace css::container;
using namespace css::sheet;
using namespace css::beans;
using namespace css::table;

SAL_IMPLEMENT_MAIN()
{
    try
    {
        Reference<XComponentContext> xRemoteContext = bootstrap();
        if (!xRemoteContext.is())
        {
            std::cerr << "ERROR: Could not bootstrap default Office.\n";
            return 1;
        }

        Reference<XMultiComponentFactory> xRemoteServiceManager
            = xRemoteContext->getServiceManager();

        Reference<XInterface> desktop = xRemoteServiceManager->createInstanceWithContext(
            "com.sun.star.frame.Desktop", xRemoteContext);
        Reference<XComponentLoader> xComponentLoader
            = Reference<XComponentLoader>(desktop, UNO_QUERY_THROW);

        Sequence<PropertyValue> loadProps(0);
        Reference<XComponent> xSpreadsheetComponent = xComponentLoader->loadComponentFromURL(
            "private:factory/scalc", "_blank", 0, loadProps);

        Reference<XSpreadsheetDocument> xSpreadsheetDocument(xSpreadsheetComponent,
                                                             UNO_QUERY_THROW);
        Reference<XSpreadsheets> xSpreadsheets = xSpreadsheetDocument->getSheets();
        xSpreadsheets->insertNewByName("MySheet", (sal_Int16)0);
        Type elemType = xSpreadsheets->getElementType();

        std::cout << elemType.getTypeName() << std::endl;
        Any sheet = xSpreadsheets->getByName("MySheet");
        Reference<XSpreadsheet> xSpreadsheet(sheet, UNO_QUERY_THROW);

        Reference<XCell> xCell = xSpreadsheet->getCellByPosition(0, 0);
        xCell->setValue(21);
        xCell = xSpreadsheet->getCellByPosition(0, 1);
        xCell->setValue(21);
        xCell = xSpreadsheet->getCellByPosition(0, 2);
        xCell->setFormula("=sum(A1:A2)");

        Reference<XPropertySet> xCellProps(xCell, UNO_QUERY_THROW);
        xCellProps->setPropertyValue("CellStyle", Any(OUString("Result")));

        Reference<XModel> xSpreadsheetModel(xSpreadsheetComponent, UNO_QUERY_THROW);
        Reference<XController> xSpreadsheetController = xSpreadsheetModel->getCurrentController();
        Reference<XSpreadsheetView> xSpreadsheetView(xSpreadsheetController, UNO_QUERY_THROW);
        xSpreadsheetView->setActiveSheet(xSpreadsheet);

        // *********************************************************
        // example for use of enum types
        xCellProps->setPropertyValue("VertJustify", Any(CellVertJustify_TOP));

        // *********************************************************
        // example for a sequence of PropertyValue structs
        // create an array with one PropertyValue struct, it contains
        // references only
        loadProps.realloc(1);

        // instantiate PropertyValue struct and set its member fields
        PropertyValue asTemplate;
        asTemplate.Name = "AsTemplate";
        asTemplate.Value = makeAny(true);

        // assign PropertyValue struct to array of references for PropertyValue
        // structs
        loadProps[0] = asTemplate;

        // load calc file as a template
        // xSpreadsheetComponent = xComponentLoader->loadComponentFromURL(
        //     "file:///c:/temp/DataAnalysys.ods", "_blank", 0, loadProps);

        // *********************************************************
        // example for use of XEnumerationAccess
        Reference<XCellRangesQuery> xCellQuery(sheet, UNO_QUERY_THROW);
        Reference<XSheetCellRanges> xFormulaCells
            = xCellQuery->queryContentCells((sal_Int16)CellFlags::FORMULA);
        Reference<XEnumerationAccess> xFormulas = xFormulaCells->getCells();
        Reference<XEnumeration> xFormulaEnum = xFormulas->createEnumeration();

        while (xFormulaEnum->hasMoreElements())
        {
            Reference<XCell> formulaCell(xFormulaEnum->nextElement(), UNO_QUERY_THROW);
            Reference<XCellAddressable> xCellAddress(formulaCell, UNO_QUERY_THROW);
            if (xCellAddress.is())
            {
                std::cout << "Formula cell in column " << xCellAddress->getCellAddress().Column
                          << ", row " << xCellAddress->getCellAddress().Row << " contains "
                          << formulaCell->getFormula() << std::endl;
            }
        }
    }
    catch (Exception& e)
    {
        std::cerr << e.Message << "\n";
        return 1;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
