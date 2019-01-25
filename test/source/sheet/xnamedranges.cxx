/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xnamedranges.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/Border.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XNamedRanges::testAddNewByName()
{
    uno::Reference<sheet::XNamedRanges> xNamedRanges(init(), UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNamedRangesNameAccess(getXNamedRanges(1),
                                                                  UNO_QUERY_THROW);

    table::CellAddress aBaseAddress = table::CellAddress(0, 0, 0);

    sal_Int32 nType = 0;
    OUString const aContent1("D1");
    OUString aName1("type_0");
    xNamedRanges->addNewByName(aName1, aContent1, aBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type 0 - Normal case",
                           xNamedRanges->hasByName(aName1));

    nType = ::sheet::NamedRangeFlag::COLUMN_HEADER;
    OUString const aContent2("D2");
    OUString aName2("type_COLUMN_HEADER");
    xNamedRanges->addNewByName(aName2, aContent2, aBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type COLUMN_HEADER",
                           xNamedRanges->hasByName(aName2));

    nType = ::sheet::NamedRangeFlag::FILTER_CRITERIA;
    OUString const aContent3("D3");
    OUString aName3("type_FILTER_CRITERIA");
    xNamedRanges->addNewByName(aName3, aContent3, aBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type FILTER_CRITERIA",
                           xNamedRanges->hasByName(aName3));

    nType = ::sheet::NamedRangeFlag::PRINT_AREA;
    OUString const aContent4("D4");
    OUString aName4("type_PRINT_AREA");
    xNamedRanges->addNewByName(aName4, aContent4, aBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type PRINT_AREA",
                           xNamedRanges->hasByName(aName4));

    nType = ::sheet::NamedRangeFlag::ROW_HEADER;
    OUString const aContent5("D5");
    OUString aName5("type_ROW_HEADER");
    xNamedRanges->addNewByName(aName5, aContent5, aBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type ROW_HEADER",
                           xNamedRanges->hasByName(aName5));
}

void XNamedRanges::testAddNewFromTitles()
{
    uno::Reference<sheet::XNamedRanges> xNamedRanges(getXNamedRanges(1), UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xNamedRangesIndex(xNamedRanges, UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNamedRangesNameAccess(xNamedRanges, UNO_QUERY_THROW);

    table::CellRangeAddress aCellRangeAddress = table::CellRangeAddress(1, 0, 0, 3, 3);

    OUString aString;
    uno::Reference<table::XCell> xCell;

    xNamedRanges->addNewFromTitles(aCellRangeAddress, sheet::Border_TOP);

    for (sal_Int32 i = 1; i < 4; i++)
    {
        // verify namedrange exists
        xCell = xSheet->getCellByPosition(i, 0);
        uno::Reference<text::XTextRange> xTextRange(xCell, UNO_QUERY_THROW);
        aString = xTextRange->getString();
        std::cout << "addNewFromTitles: verify " << aString << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Non existing NamedRange", xNamedRanges->hasByName(aString));

        // verify it points on the right cell
        uno::Any aNr = xNamedRangesNameAccess->getByName(aString);
        uno::Reference<sheet::XNamedRange> xNamedRange(aNr, UNO_QUERY_THROW);

        uno::Reference<sheet::XCellRangeReferrer> xCellRangeRef(xNamedRange, UNO_QUERY_THROW);
        uno::Reference<sheet::XCellRangeAddressable> xCellRangeAdr(
            xCellRangeRef->getReferredCells(), UNO_QUERY_THROW);
        table::CellRangeAddress cellRangeAddress = xCellRangeAdr->getRangeAddress();

        CPPUNIT_ASSERT_EQUAL(i, cellRangeAddress.EndColumn);
        CPPUNIT_ASSERT_EQUAL(i, cellRangeAddress.StartColumn);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), cellRangeAddress.StartRow);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), cellRangeAddress.EndRow);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1), cellRangeAddress.Sheet);
    }

    xNamedRanges->addNewFromTitles(aCellRangeAddress, sheet::Border_LEFT);

    for (sal_Int32 i = 1; i < 4; i++)
    {
        // verify namedrange exists
        xCell = xSheet->getCellByPosition(0, i);
        uno::Reference<text::XTextRange> xTextRange(xCell, UNO_QUERY_THROW);
        aString = xTextRange->getString();
        std::cout << "verify " << aString << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Non existing NamedRange", xNamedRanges->hasByName(aString));

        // verify it points on the right cell
        uno::Any aNr = xNamedRangesNameAccess->getByName(aString);
        uno::Reference<sheet::XNamedRange> xNamedRange(aNr, UNO_QUERY_THROW);

        uno::Reference<sheet::XCellRangeReferrer> xCellRangeRef(xNamedRange, UNO_QUERY_THROW);
        uno::Reference<sheet::XCellRangeAddressable> xCellRangeAdr(
            xCellRangeRef->getReferredCells(), UNO_QUERY_THROW);
        table::CellRangeAddress cellRangeAddress = xCellRangeAdr->getRangeAddress();

        CPPUNIT_ASSERT_EQUAL(i, cellRangeAddress.EndRow);
        CPPUNIT_ASSERT_EQUAL(i, cellRangeAddress.StartRow);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), cellRangeAddress.StartColumn);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), cellRangeAddress.EndColumn);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1), cellRangeAddress.Sheet);
    }
}

void XNamedRanges::testRemoveByName()
{
    uno::Reference<sheet::XNamedRanges> xNamedRanges(init(), UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xNamedRanges, UNO_QUERY_THROW);

    bool bHasIt = xNamedRanges->hasByName(maNameToRemove);
    CPPUNIT_ASSERT_MESSAGE("NamedRange initial1 does not exist, can't remove it", bHasIt);

    if (bHasIt)
    {
        // remove existing
        sal_Int32 nInitialCount = xIndex->getCount();
        xNamedRanges->removeByName(maNameToRemove);
        sal_Int32 nNewCount = xIndex->getCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("NamedRange initial1 not removed", nNewCount,
                                     nInitialCount - 1);
        CPPUNIT_ASSERT_MESSAGE("Wrong NamedRange removed, initial1 still present",
                               !xNamedRanges->hasByName(maNameToRemove));
        // try to remove non existing
        xNamedRanges->removeByName(
            "dummyNonExistingNamedRange"); // an exception should be raised here
    }
}

void XNamedRanges::testOutputList()
{
    table::CellAddress aCellAddress = table::CellAddress(0, 2, 0);
    uno::Reference<sheet::XNamedRanges> xNamedRanges(init(), UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xNamedRangesIndex(init(), UNO_QUERY_THROW);
    sal_Int32 nElementsCount = xNamedRangesIndex->getCount();

    xNamedRanges->outputList(aCellAddress);

    OUString aString;
    uno::Reference<table::XCell> xCell;

    for (sal_Int32 i = 0; i < nElementsCount; i++)
    {
        xCell = xSheet->getCellByPosition(2, i);
        uno::Reference<text::XTextRange> xTextRange(xCell, UNO_QUERY_THROW);
        aString = xTextRange->getString();
        std::cout << "verify " << aString << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Non existing NamedRange", xNamedRanges->hasByName(aString));
    }
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
