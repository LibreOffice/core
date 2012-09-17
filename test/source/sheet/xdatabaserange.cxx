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

using namespace com::sun::star::uno;

namespace apitest {

/**
 * tests setDataArea and getDataArea
 */
void XDatabaseRange::testDataArea()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange(init(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataArea"))), UNO_QUERY_THROW);

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
    uno::Reference< sheet::XDatabaseRange > xDBRange(init(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SubtotalDescriptor"))), UNO_QUERY_THROW);
    uno::Reference< sheet::XSubTotalDescriptor> xSubtotalDescr = xDBRange->getSubTotalDescriptor();
    CPPUNIT_ASSERT(xSubtotalDescr.is());
}

void XDatabaseRange::testGetSortDescriptor()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange(init(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SortDescriptor"))), UNO_QUERY_THROW);
    uno::Sequence< beans::PropertyValue > xSortDescr = xDBRange->getSortDescriptor();
    for (sal_Int32 i = 0; i < xSortDescr.getLength(); ++i)
    {
        beans::PropertyValue xProp = xSortDescr[i];
        //std::cout << "Prop " << i << " Name: " << rtl::OUString(xProp.Name) << std::endl;

        if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsSortColumns")))
        {
            sal_Bool bIsSortColumns = sal_True;
            xProp.Value >>= bIsSortColumns;
            CPPUNIT_ASSERT(bIsSortColumns == sal_True);
        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ContainsHeader")))
        {
            sal_Bool bContainsHeader = sal_True;
            xProp.Value >>= bContainsHeader;
            CPPUNIT_ASSERT(bContainsHeader == sal_True);
        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MaxFieldCount")))
        {
            sal_Int32 nMaxFieldCount = 0;
            xProp.Value >>= nMaxFieldCount;
            std::cout << "Value: " << nMaxFieldCount << std::endl;

        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SortFields")))
        {

        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BindFormatsToContent")))
        {
            sal_Bool bBindFormatsToContent = sal_False;
            xProp.Value >>= bBindFormatsToContent;
            CPPUNIT_ASSERT(bBindFormatsToContent == sal_True);
        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CopyOutputData")))
        {
            sal_Bool bCopyOutputData = sal_True;
            xProp.Value >>= bCopyOutputData;
            CPPUNIT_ASSERT(bCopyOutputData == sal_False);
        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OutputPosition")))
        {

        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsUserListEnabled")))
        {
            sal_Bool bIsUserListEnabled  = sal_True;
            xProp.Value >>= bIsUserListEnabled;
            CPPUNIT_ASSERT(bIsUserListEnabled == sal_False);

        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UserListIndex")))
        {
            sal_Int32 nUserListIndex = 1;
            xProp.Value >>= nUserListIndex;
            CPPUNIT_ASSERT(nUserListIndex == 0);
        }
    }
}

void XDatabaseRange::testGetFilterDescriptor()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange( init(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterDescriptor"))), UNO_QUERY_THROW);
    uno::Reference< uno::XInterface > xFilterDescr( xDBRange->getFilterDescriptor(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xFilterDescr.is());
}

void XDatabaseRange::testGetImportDescriptor()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange( init(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ImportDescriptor"))), UNO_QUERY_THROW);
    uno::Sequence< beans::PropertyValue > xImportDescr = xDBRange->getImportDescriptor();
}

void XDatabaseRange::testRefresh()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange( init(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Refresh"))), UNO_QUERY_THROW);

    const sal_Int32 nCol = 0;
    rtl::OUString aHidden(RTL_CONSTASCII_USTRINGPARAM("IsVisible"));
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
