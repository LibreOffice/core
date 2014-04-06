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
    OUString aTestedNamedRangeString("initial1");
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(aTestedNamedRangeString);

    OUString aExpectedContent("$Sheet1.$B$1");
    CPPUNIT_ASSERT_MESSAGE("Wrong expected content for initial1 on GetContent", xNamedRange->getContent().equals(aExpectedContent));
}

void XNamedRange::testSetContent()
{
    OUString aTestedNamedRangeString("initial1");
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(aTestedNamedRangeString);

    OUString aExpectedContent;

    // test a cell
    aExpectedContent = "D1";
    xNamedRange->setContent(aExpectedContent);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected content for initial1 after SetContent a cell", xNamedRange->getContent().equals(aExpectedContent));

    // test a cellrange
    aExpectedContent = "D1:D10";
    xNamedRange->setContent(aExpectedContent);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected content for initial1 after SetContent a cellrange", xNamedRange->getContent().equals(aExpectedContent));

    // test a formula
    aExpectedContent = "=D10";
    xNamedRange->setContent(aExpectedContent);
    aExpectedContent = "D10";
    CPPUNIT_ASSERT_MESSAGE("Wrong expected content for initial1 after SetContent a formula", xNamedRange->getContent().equals(aExpectedContent));

}

void XNamedRange::testGetType()
{
    OUString aTestedNamedRangeString("initial1");
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(aTestedNamedRangeString);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected Type", xNamedRange->getType() == 0);
}

void XNamedRange::testSetType()
{
    OUString aTestedNamedRangeString("initial1");
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(aTestedNamedRangeString);

    sal_Int32 nType = ::sheet::NamedRangeFlag::ROW_HEADER;;
    xNamedRange->setType(nType);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected Type ROW_HEADER after setting it", xNamedRange->getType() == nType);

    nType = ::sheet::NamedRangeFlag::COLUMN_HEADER;
    xNamedRange->setType(nType);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected Type COLUMN_HEADER after setting it", xNamedRange->getType() == nType);

    nType = ::sheet::NamedRangeFlag::FILTER_CRITERIA;
    xNamedRange->setType(nType);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected Type FILTER_CRITERIA after setting it", xNamedRange->getType() == nType);

    nType = ::sheet::NamedRangeFlag::PRINT_AREA;
    xNamedRange->setType(nType);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected Type PRINT_AREA after setting it", xNamedRange->getType() == nType);

    nType = 0;
    xNamedRange->setType(nType);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected Type 0 after setting it", xNamedRange->getType() == nType);
}

void XNamedRange::testGetReferencePosition()
{
    OUString aTestedNamedRangeString("initial2");
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(aTestedNamedRangeString);

    table::CellAddress xCellAddress = xNamedRange->getReferencePosition();
    // the expeted address is on B1, as it was the active cell when intial2 created
    CPPUNIT_ASSERT_MESSAGE("Wrong SHEET reference position", xCellAddress.Sheet == 0);
    CPPUNIT_ASSERT_MESSAGE("Wrong COLUMN reference position", xCellAddress.Column == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong ROW reference position", xCellAddress.Row == 0);
}

void XNamedRange::testSetReferencePosition()
{
    OUString aTestedNamedRangeString("initial1");
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(aTestedNamedRangeString);

    table::CellAddress aBaseAddress = table::CellAddress(1,2,3);

    xNamedRange->setReferencePosition(aBaseAddress);

    table::CellAddress xCellAddress = xNamedRange->getReferencePosition();
    CPPUNIT_ASSERT_MESSAGE("Wrong SHEET reference position after setting it", xCellAddress.Sheet == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong COLUMN reference position after setting it", xCellAddress.Column == 2);
    CPPUNIT_ASSERT_MESSAGE("Wrong ROW reference position after setting it", xCellAddress.Row == 3);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
