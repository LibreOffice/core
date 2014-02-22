/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <test/sheet/xdatabaserange.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>

#include <rtl/ustring.hxx>
#include "cppunit/extensions/HelperMacros.h"
#include <iostream>

using namespace css;
using namespace css::uno;

namespace apitest {

/**
 * tests setDataArea and getDataArea
 */
void XDatabaseRange::testDataArea()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange(init(OUString("DataArea")), UNO_QUERY_THROW);

    table::CellRangeAddress aCellAddress;
    aCellAddress.Sheet = 0;
    aCellAddress.StartColumn = 1;
    aCellAddress.EndColumn = 4;
    aCellAddress.StartRow = 2;
    aCellAddress.EndRow = 5;
    xDBRange->setDataArea(aCellAddress);
    table::CellRangeAddress aValue;
    aValue = xDBRange->getDataArea();
    CPPUNIT_ASSERT( aCellAddress.Sheet == aValue.Sheet );
    CPPUNIT_ASSERT( aCellAddress.StartRow == aValue.StartRow );
    CPPUNIT_ASSERT( aCellAddress.EndRow == aValue.EndRow );
    CPPUNIT_ASSERT( aCellAddress.StartColumn == aValue.StartColumn );
    CPPUNIT_ASSERT( aCellAddress.EndColumn == aValue.EndColumn );
}

void XDatabaseRange::testGetSubtotalDescriptor()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange(init(OUString("SubtotalDescriptor")), UNO_QUERY_THROW);
    uno::Reference< sheet::XSubTotalDescriptor> xSubtotalDescr = xDBRange->getSubTotalDescriptor();
    CPPUNIT_ASSERT(xSubtotalDescr.is());
}

void XDatabaseRange::testGetSortDescriptor()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange(init(OUString("SortDescriptor")), UNO_QUERY_THROW);
    uno::Sequence< beans::PropertyValue > xSortDescr = xDBRange->getSortDescriptor();
    for (sal_Int32 i = 0; i < xSortDescr.getLength(); ++i)
    {
        beans::PropertyValue xProp = xSortDescr[i];
        

        if (xProp.Name == "IsSortColumns")
        {
            sal_Bool bIsSortColumns = sal_True;
            xProp.Value >>= bIsSortColumns;
            CPPUNIT_ASSERT(bIsSortColumns == sal_True);
        }
        else if (xProp.Name == "ContainsHeader")
        {
            sal_Bool bContainsHeader = sal_True;
            xProp.Value >>= bContainsHeader;
            CPPUNIT_ASSERT(bContainsHeader == sal_True);
        }
        else if (xProp.Name == "MaxFieldCount")
        {
            sal_Int32 nMaxFieldCount = 0;
            xProp.Value >>= nMaxFieldCount;
            std::cout << "Value: " << nMaxFieldCount << std::endl;

        }
        else if (xProp.Name == "SortFields")
        {

        }
        else if (xProp.Name == "BindFormatsToContent")
        {
            sal_Bool bBindFormatsToContent = sal_False;
            xProp.Value >>= bBindFormatsToContent;
            CPPUNIT_ASSERT(bBindFormatsToContent == sal_True);
        }
        else if (xProp.Name == "CopyOutputData")
        {
            sal_Bool bCopyOutputData = sal_True;
            xProp.Value >>= bCopyOutputData;
            CPPUNIT_ASSERT(bCopyOutputData == sal_False);
        }
        else if (xProp.Name == "OutputPosition")
        {

        }
        else if (xProp.Name == "IsUserListEnabled")
        {
            sal_Bool bIsUserListEnabled  = sal_True;
            xProp.Value >>= bIsUserListEnabled;
            CPPUNIT_ASSERT(bIsUserListEnabled == sal_False);

        }
        else if (xProp.Name == "UserListIndex")
        {
            sal_Int32 nUserListIndex = 1;
            xProp.Value >>= nUserListIndex;
            CPPUNIT_ASSERT(nUserListIndex == 0);
        }
    }
}

void XDatabaseRange::testGetFilterDescriptor()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange( init(OUString("FilterDescriptor")), UNO_QUERY_THROW);
    uno::Reference< uno::XInterface > xFilterDescr( xDBRange->getFilterDescriptor(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xFilterDescr.is());
}

void XDatabaseRange::testGetImportDescriptor()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange( init(OUString("ImportDescriptor")), UNO_QUERY_THROW);
    uno::Sequence< beans::PropertyValue > xImportDescr = xDBRange->getImportDescriptor();
    (void) xImportDescr;
}

void XDatabaseRange::testRefresh()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange( init(OUString("Refresh")), UNO_QUERY_THROW);

    const sal_Int32 nCol = 0;
    OUString aHidden("IsVisible");
    uno::Reference< sheet::XCellRangeReferrer > xCellRangeReferrer(xDBRange, UNO_QUERY_THROW);
    uno::Reference< table::XCellRange > xCellRange = xCellRangeReferrer->getReferredCells();

    for (sal_Int32 i = 1; i < 5; ++i)
    {
        uno::Reference< table::XCell > xCell = xCellRange->getCellByPosition(nCol, i);
        xCell->setValue(0);
    }

    for (sal_Int32 i = 2; i < 5; ++i)
    {
        uno::Reference< table::XColumnRowRange > xColRowRange(xCellRange, UNO_QUERY_THROW);
        uno::Reference< table::XTableRows > xRows = xColRowRange->getRows();
        uno::Reference< table::XCellRange > xRow(xRows->getByIndex(i), UNO_QUERY_THROW);
        uno::Reference< beans::XPropertySet > xPropRow(xRow, UNO_QUERY_THROW);
        Any aAny = xPropRow->getPropertyValue( aHidden );

        CPPUNIT_ASSERT(aAny.get<sal_Bool>() == sal_True);
    }

    xDBRange->refresh();
    std::cout << "after refresh" << std::endl;

    for (sal_Int32 i = 1; i < 5; ++i)
    {
        uno::Reference< table::XColumnRowRange > xColRowRange(xCellRange, UNO_QUERY_THROW);
        uno::Reference< table::XTableRows > xRows = xColRowRange->getRows();
        uno::Reference< table::XCellRange > xRow(xRows->getByIndex(i), UNO_QUERY_THROW);
        uno::Reference< beans::XPropertySet > xPropRow(xRow, UNO_QUERY_THROW);
        Any aAny = xPropRow->getPropertyValue( aHidden );

        CPPUNIT_ASSERT(aAny.get<sal_Bool>() == sal_False);
    }


}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
