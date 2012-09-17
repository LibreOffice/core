/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

using namespace com::sun::star::uno;

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
