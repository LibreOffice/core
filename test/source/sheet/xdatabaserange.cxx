/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xdatabaserange.hxx>

#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>

#include <rtl/ustring.hxx>
#include <cppunit/TestAssert.h>
#include <iostream>

using namespace css;
using namespace css::uno;

namespace apitest
{
/**
 * tests setDataArea and getDataArea
 */
void XDatabaseRange::testDataArea()
{
    uno::Reference<sheet::XDatabaseRange> xDBRange(init(u"DataArea"_ustr), UNO_QUERY_THROW);

    table::CellRangeAddress aCellAddress;
    aCellAddress.Sheet = 0;
    aCellAddress.StartColumn = 1;
    aCellAddress.EndColumn = 4;
    aCellAddress.StartRow = 2;
    aCellAddress.EndRow = 5;
    xDBRange->setDataArea(aCellAddress);
    table::CellRangeAddress aValue = xDBRange->getDataArea();
    CPPUNIT_ASSERT_EQUAL(aCellAddress.Sheet, aValue.Sheet);
    CPPUNIT_ASSERT_EQUAL(aCellAddress.StartRow, aValue.StartRow);
    CPPUNIT_ASSERT_EQUAL(aCellAddress.EndRow, aValue.EndRow);
    CPPUNIT_ASSERT_EQUAL(aCellAddress.StartColumn, aValue.StartColumn);
    CPPUNIT_ASSERT_EQUAL(aCellAddress.EndColumn, aValue.EndColumn);
}

void XDatabaseRange::testGetSubtotalDescriptor()
{
    uno::Reference<sheet::XDatabaseRange> xDBRange(init(u"SubtotalDescriptor"_ustr),
                                                   UNO_QUERY_THROW);
    uno::Reference<sheet::XSubTotalDescriptor> xSubtotalDescr = xDBRange->getSubTotalDescriptor();
    CPPUNIT_ASSERT(xSubtotalDescr.is());
}

void XDatabaseRange::testGetSortDescriptor()
{
    uno::Reference<sheet::XDatabaseRange> xDBRange(init(u"SortDescriptor"_ustr), UNO_QUERY_THROW);
    const uno::Sequence<beans::PropertyValue> xSortDescr = xDBRange->getSortDescriptor();
    for (const beans::PropertyValue& aProp : xSortDescr)
    {
        //std::cout << "Prop " << i << " Name: " << OUString(aProp.Name) << std::endl;

        if (aProp.Name == "IsSortColumns")
        {
            bool bIsSortColumns = true;
            aProp.Value >>= bIsSortColumns;
            CPPUNIT_ASSERT(!bIsSortColumns);
        }
        else if (aProp.Name == "ContainsHeader")
        {
            bool bContainsHeader = true;
            aProp.Value >>= bContainsHeader;
            CPPUNIT_ASSERT(bContainsHeader);
        }
        else if (aProp.Name == "MaxFieldCount")
        {
            sal_Int32 nMaxFieldCount = 0;
            aProp.Value >>= nMaxFieldCount;
            std::cout << "Value: " << nMaxFieldCount << std::endl;
        }
        else if (aProp.Name == "SortFields")
        {
        }
        else if (aProp.Name == "BindFormatsToContent")
        {
            bool bBindFormatsToContent = false;
            aProp.Value >>= bBindFormatsToContent;
            CPPUNIT_ASSERT(bBindFormatsToContent);
        }
        else if (aProp.Name == "CopyOutputData")
        {
            bool bCopyOutputData = true;
            aProp.Value >>= bCopyOutputData;
            CPPUNIT_ASSERT(!bCopyOutputData);
        }
        else if (aProp.Name == "OutputPosition")
        {
        }
        else if (aProp.Name == "IsUserListEnabled")
        {
            bool bIsUserListEnabled = true;
            aProp.Value >>= bIsUserListEnabled;
            CPPUNIT_ASSERT(!bIsUserListEnabled);
        }
        else if (aProp.Name == "UserListIndex")
        {
            sal_Int32 nUserListIndex = 1;
            aProp.Value >>= nUserListIndex;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nUserListIndex);
        }
    }
}

void XDatabaseRange::testGetFilterDescriptor()
{
    uno::Reference<sheet::XDatabaseRange> xDBRange(init(u"FilterDescriptor"_ustr), UNO_QUERY_THROW);
    uno::Reference<uno::XInterface> xFilterDescr(xDBRange->getFilterDescriptor(), UNO_QUERY_THROW);
}

void XDatabaseRange::testGetImportDescriptor()
{
    uno::Reference<sheet::XDatabaseRange> xDBRange(init(u"ImportDescriptor"_ustr), UNO_QUERY_THROW);
    (void)xDBRange->getImportDescriptor();
}

void XDatabaseRange::testRefresh()
{
    uno::Reference<sheet::XDatabaseRange> xDBRange(init(u"Refresh"_ustr), UNO_QUERY_THROW);

    const sal_Int32 nCol = 0;
    OUString aHidden(u"IsVisible"_ustr);
    uno::Reference<sheet::XCellRangeReferrer> xCellRangeReferrer(xDBRange, UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCellRange = xCellRangeReferrer->getReferredCells();

    for (sal_Int32 i = 1; i < 5; ++i)
    {
        uno::Reference<table::XCell> xCell = xCellRange->getCellByPosition(nCol, i);
        xCell->setValue(0);
    }

    for (sal_Int32 i = 2; i < 5; ++i)
    {
        uno::Reference<table::XColumnRowRange> xColRowRange(xCellRange, UNO_QUERY_THROW);
        uno::Reference<table::XTableRows> xRows = xColRowRange->getRows();
        uno::Reference<table::XCellRange> xRow(xRows->getByIndex(i), UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropRow(xRow, UNO_QUERY_THROW);
        Any aAny = xPropRow->getPropertyValue(aHidden);

        CPPUNIT_ASSERT(aAny.get<bool>());
    }

    xDBRange->refresh();
    std::cout << "after refresh" << std::endl;

    for (sal_Int32 i = 1; i < 5; ++i)
    {
        uno::Reference<table::XColumnRowRange> xColRowRange(xCellRange, UNO_QUERY_THROW);
        uno::Reference<table::XTableRows> xRows = xColRowRange->getRows();
        uno::Reference<table::XCellRange> xRow(xRows->getByIndex(i), UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropRow(xRow, UNO_QUERY_THROW);
        Any aAny = xPropRow->getPropertyValue(aHidden);

        CPPUNIT_ASSERT(!aAny.get<bool>());
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
