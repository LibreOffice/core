/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xnamedrange.hxx>

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

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/Border.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>

#include "cppunit/extensions/HelperMacros.h"
#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

void XNamedRange::testGetContent()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange("initial1");

    OUString const aExpectedContent("$Sheet1.$B$1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected content for initial1 on GetContent", xNamedRange->getContent(), aExpectedContent);
}

void XNamedRange::testSetContent()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange("initial1");

    OUString aExpectedContent;

    // test a cell
    aExpectedContent = "D1";
    xNamedRange->setContent(aExpectedContent);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected content for initial1 after SetContent a cell", xNamedRange->getContent(), aExpectedContent);

    // test a cellrange
    aExpectedContent = "D1:D10";
    xNamedRange->setContent(aExpectedContent);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected content for initial1 after SetContent a cellrange", xNamedRange->getContent(), aExpectedContent);

    // test a formula
    aExpectedContent = "=D10";
    xNamedRange->setContent(aExpectedContent);
    aExpectedContent = "D10";
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected content for initial1 after SetContent a formula", xNamedRange->getContent(), aExpectedContent);
}

void XNamedRange::testGetType()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange("initial1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected Type", sal_Int32(0), xNamedRange->getType());
}

void XNamedRange::testSetType()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange("initial1");

    sal_Int32 nType = ::sheet::NamedRangeFlag::ROW_HEADER;
    xNamedRange->setType(nType);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected Type ROW_HEADER after setting it", nType, xNamedRange->getType());

    nType = ::sheet::NamedRangeFlag::COLUMN_HEADER;
    xNamedRange->setType(nType);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected Type COLUMN_HEADER after setting it", nType, xNamedRange->getType());

    nType = ::sheet::NamedRangeFlag::FILTER_CRITERIA;
    xNamedRange->setType(nType);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected Type FILTER_CRITERIA after setting it", nType, xNamedRange->getType());

    nType = ::sheet::NamedRangeFlag::PRINT_AREA;
    xNamedRange->setType(nType);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected Type PRINT_AREA after setting it", nType, xNamedRange->getType());

    nType = 0;
    xNamedRange->setType(nType);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected Type 0 after setting it", nType, xNamedRange->getType());
}

void XNamedRange::testGetReferencePosition()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange("initial2");

    table::CellAddress aCellAddress = xNamedRange->getReferencePosition();
    // the expected address is on B1, as it was the active cell when initial2 was created
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong SHEET reference position", sal_Int16(0), aCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong COLUMN reference position", sal_Int32(1), aCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ROW reference position", sal_Int32(0), aCellAddress.Row);
}

void XNamedRange::testSetReferencePosition()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange("initial1");

    table::CellAddress aBaseAddress = table::CellAddress(1,2,3);

    xNamedRange->setReferencePosition(aBaseAddress);

    table::CellAddress aCellAddress = xNamedRange->getReferencePosition();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong SHEET reference position after setting it", sal_Int16(1), aCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong COLUMN reference position after setting it", sal_Int32(2), aCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ROW reference position after setting it", sal_Int32(3), aCellAddress.Row);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
