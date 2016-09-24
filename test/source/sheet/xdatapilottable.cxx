/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xdatapilottable.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest
{

void XDataPilotTable::testGetOutputRange()
{
    uno::Reference< sheet::XDataPilotTable > xDPTable(init(),UNO_QUERY_THROW);

    table::CellRangeAddress aRange = xDPTable->getOutputRange();
    CPPUNIT_ASSERT( aRange.Sheet == 0 );
    CPPUNIT_ASSERT( aRange.StartColumn == 7 );
    CPPUNIT_ASSERT( aRange.StartRow == 8 );
}

void XDataPilotTable::testRefresh()
{
    uno::Reference< sheet::XDataPilotTable > xDPTable(init(),UNO_QUERY_THROW);
    std::cout << "xCellForChange: Old Value: " << xCellForChange->getValue() << std::endl;
    std::cout << "xCellForCheck: Old Value: " << xCellForCheck->getValue() << std::endl;
    double aOldData = xCellForCheck->getValue();
    xCellForChange->setValue( 5 );

    xDPTable->refresh();
    xDPTable->refresh();
    std::cout << "xCellForChange: Old Value: " << xCellForChange->getValue() << std::endl;
    std::cout << "xCellForCheck: Old Value: " << xCellForCheck->getValue() << std::endl;
    double aNewData = xCellForCheck->getValue();
    CPPUNIT_ASSERT_MESSAGE("value needs to change", aOldData != aNewData);
}

XDataPilotTable::~XDataPilotTable()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
