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
 * Copyright (C) 2011 Laurent Godard lgodard.libre@laposte.net (initial developer)
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

#include <test/sheet/xnamedranges.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/Border.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>

#include <rtl/ustring.hxx>
#include "cppunit/extensions/HelperMacros.h"
#include <iostream>

using namespace com::sun::star::uno;

namespace apitest {

XNamedRanges::XNamedRanges():
    maNameToRemove(RTL_CONSTASCII_USTRINGPARAM("initial1"))
{

}

XNamedRanges::XNamedRanges(const rtl::OUString& rNameToRemove):
    maNameToRemove(rNameToRemove)
{

}

XNamedRanges::~XNamedRanges()
{
}

void XNamedRanges::testAddNewByName()
{
    uno::Reference< sheet::XNamedRanges > xNamedRanges(init(), UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xNamedRangesNameAccess(init(1), UNO_QUERY_THROW);

    table::CellAddress aBaseAddress = table::CellAddress(0,0,0);

    sal_Int32 nType = 0;
    rtl::OUString aContent1(RTL_CONSTASCII_USTRINGPARAM("D1"));
    rtl::OUString aName1(RTL_CONSTASCII_USTRINGPARAM("type_0"));
    xNamedRanges->addNewByName(aName1, aContent1, aBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type 0 - Normal case", xNamedRanges->hasByName(aName1));

    nType = ::sheet::NamedRangeFlag::COLUMN_HEADER;
    rtl::OUString aContent2(RTL_CONSTASCII_USTRINGPARAM("D2"));
    rtl::OUString aName2(RTL_CONSTASCII_USTRINGPARAM("type_COLUMN_HEADER"));
    xNamedRanges->addNewByName(aName2, aContent2, aBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type COLUMN_HEADER", xNamedRanges->hasByName(aName2));

    nType = ::sheet::NamedRangeFlag::FILTER_CRITERIA;
    rtl::OUString aContent3(RTL_CONSTASCII_USTRINGPARAM("D3"));
    rtl::OUString aName3(RTL_CONSTASCII_USTRINGPARAM("type_FILTER_CRITERIA"));
    xNamedRanges->addNewByName(aName3, aContent3, aBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type FILTER_CRITERIA", xNamedRanges->hasByName(aName3));

    nType = ::sheet::NamedRangeFlag::PRINT_AREA;
    rtl::OUString aContent4(RTL_CONSTASCII_USTRINGPARAM("D4"));
    rtl::OUString aName4(RTL_CONSTASCII_USTRINGPARAM("type_PRINT_AREA"));
    xNamedRanges->addNewByName(aName4, aContent4, aBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type PRINT_AREA", xNamedRanges->hasByName(aName4));

    nType = ::sheet::NamedRangeFlag::ROW_HEADER;
    rtl::OUString aContent5(RTL_CONSTASCII_USTRINGPARAM("D5"));
    rtl::OUString aName5(RTL_CONSTASCII_USTRINGPARAM("type_ROW_HEADER"));
    xNamedRanges->addNewByName(aName5, aContent5, aBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type ROW_HEADER", xNamedRanges->hasByName(aName5));

}

void XNamedRanges::testAddNewFromTitles()
{
    uno::Reference< sheet::XNamedRanges > xNamedRanges(init(1), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xNamedRangesIndex(xNamedRanges, UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xNamedRangesNameAccess(xNamedRanges, UNO_QUERY_THROW);

    table::CellRangeAddress aCellRangeAddress = table::CellRangeAddress(1,0,0,3,3);

    rtl::OUString aString;
    uno::Reference< table::XCell > xCell;

    xNamedRanges->addNewFromTitles(aCellRangeAddress, sheet::Border_TOP);

    for (sal_Int32 i = 1; i < 4; i++)
    {
        // verify namedrange exists
        xCell = xSheet->getCellByPosition(i,0);
        uno::Reference< text::XTextRange > xTextRange(xCell, UNO_QUERY_THROW);
        aString = xTextRange->getString();
        std::cout << "addNewFromTitles: verify " << aString << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Non existing NamedRange", xNamedRanges->hasByName(aString));

        // verify it points on the right cell
        uno::Any aNr = xNamedRangesNameAccess->getByName(aString);
        uno::Reference< sheet::XNamedRange > xNamedRange(aNr, UNO_QUERY_THROW);

        uno::Reference< sheet::XCellRangeReferrer > xCellRangeRef(xNamedRange, UNO_QUERY_THROW);
        uno::Reference< sheet::XCellRangeAddressable > xCellRangeAdr(xCellRangeRef->getReferredCells(), UNO_QUERY_THROW);
        table::CellRangeAddress cellRangeAddress = xCellRangeAdr->getRangeAddress();

        CPPUNIT_ASSERT(cellRangeAddress.EndColumn == i && cellRangeAddress.StartColumn == i);
        CPPUNIT_ASSERT(cellRangeAddress.StartRow == 1);
        CPPUNIT_ASSERT(cellRangeAddress.EndRow == 3);
        CPPUNIT_ASSERT(cellRangeAddress.Sheet == 1);
    }

    xNamedRanges->addNewFromTitles(aCellRangeAddress, sheet::Border_LEFT);

    for (sal_Int32 i = 1; i < 4; i++)
    {
        // verify namedrange exists
        xCell = xSheet->getCellByPosition(0,i);
        uno::Reference< text::XTextRange > xTextRange(xCell, UNO_QUERY_THROW);
        aString = xTextRange->getString();
        std::cout << "verify " << aString << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Non existing NamedRange", xNamedRanges->hasByName(aString));

        // verify it points on the right cell
        uno::Any aNr= xNamedRangesNameAccess->getByName(aString);
        uno::Reference< sheet::XNamedRange > xNamedRange(aNr, UNO_QUERY_THROW);

        uno::Reference< sheet::XCellRangeReferrer > xCellRangeRef(xNamedRange, UNO_QUERY_THROW);
        uno::Reference< sheet::XCellRangeAddressable > xCellRangeAdr(xCellRangeRef->getReferredCells(), UNO_QUERY_THROW);
        table::CellRangeAddress cellRangeAddress = xCellRangeAdr->getRangeAddress();

        CPPUNIT_ASSERT(cellRangeAddress.EndRow == i && cellRangeAddress.StartRow == i);
        CPPUNIT_ASSERT(cellRangeAddress.StartColumn == 1);
        CPPUNIT_ASSERT(cellRangeAddress.EndColumn == 3);
        CPPUNIT_ASSERT(cellRangeAddress.Sheet == 1);
    }
}

void XNamedRanges::testRemoveByName()
{
    uno::Reference< sheet::XNamedRanges > xNamedRanges(init(), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex(xNamedRanges, UNO_QUERY_THROW);

    bool bHasIt = xNamedRanges->hasByName(maNameToRemove);
    CPPUNIT_ASSERT_MESSAGE("NamedRange initial1 des not exits, can't remove it", bHasIt);

    if (bHasIt)
    {
        // remove existing
        sal_Int32 nInitialCount = xIndex->getCount();
        xNamedRanges->removeByName(maNameToRemove);
        sal_Int32 nNewCount = xIndex->getCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("NamedRange initial1 not removed", nNewCount, nInitialCount - 1);
        CPPUNIT_ASSERT_MESSAGE("Wrong NamedRange removed, initial1 still present", !xNamedRanges->hasByName(maNameToRemove));
        // try to remove non existing
        rtl::OUString aNr2(RTL_CONSTASCII_USTRINGPARAM("dummyNonExistingNamedRange"));
        xNamedRanges->removeByName(aNr2);// an exception should be raised here
    }
}

void XNamedRanges::testOutputList()
{
    table::CellAddress xCellAddress = table::CellAddress (0,2,0);
    uno::Reference< sheet::XNamedRanges > xNamedRanges(init(), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xNamedRangesIndex(init(), UNO_QUERY_THROW);
    sal_Int32 nElementsCount = xNamedRangesIndex->getCount();

    xNamedRanges->outputList(xCellAddress);

    rtl::OUString aString;
    uno::Reference< table::XCell > xCell;

    for (sal_Int32 i = 0; i < nElementsCount; i++)
    {
        xCell = xSheet->getCellByPosition(2,i);
        uno::Reference< text::XTextRange > xTextRange(xCell, UNO_QUERY_THROW);
        aString = xTextRange->getString();
        std::cout << "verify " << aString << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Non existing NamedRange", xNamedRanges->hasByName(aString));
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
