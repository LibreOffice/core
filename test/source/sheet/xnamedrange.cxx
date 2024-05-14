/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xnamedrange.hxx>

#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>

#include <cppunit/TestAssert.h>
#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

void XNamedRange::testGetContent()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(u"initial1"_ustr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected content for initial1 on GetContent", u"$Sheet1.$B$1"_ustr, xNamedRange->getContent());
}

void XNamedRange::testSetContent()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(u"initial1"_ustr);

    OUString aExpectedContent;

    // test a cell
    aExpectedContent = "D1";
    xNamedRange->setContent(aExpectedContent);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected content for initial1 after SetContent a cell", aExpectedContent, xNamedRange->getContent());

    // test a cellrange
    aExpectedContent = "D1:D10";
    xNamedRange->setContent(aExpectedContent);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected content for initial1 after SetContent a cellrange", aExpectedContent, xNamedRange->getContent());

    // test a formula
    aExpectedContent = "=D10";
    xNamedRange->setContent(aExpectedContent);
    aExpectedContent = "D10";
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected content for initial1 after SetContent a formula", aExpectedContent, xNamedRange->getContent());
}

void XNamedRange::testGetType()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(u"initial1"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong expected Type", sal_Int32(0), xNamedRange->getType());
}

void XNamedRange::testSetType()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(u"initial1"_ustr);

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
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(u"initial2"_ustr);

    table::CellAddress aCellAddress = xNamedRange->getReferencePosition();
    // the expected address is on B1, as it was the active cell when initial2 was created
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong SHEET reference position", sal_Int16(0), aCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong COLUMN reference position", sal_Int32(1), aCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ROW reference position", sal_Int32(0), aCellAddress.Row);
}

void XNamedRange::testSetReferencePosition()
{
    uno::Reference< sheet::XNamedRange > xNamedRange = getNamedRange(u"initial1"_ustr);

    table::CellAddress aBaseAddress(1,2,3);

    xNamedRange->setReferencePosition(aBaseAddress);

    table::CellAddress aCellAddress = xNamedRange->getReferencePosition();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong SHEET reference position after setting it", sal_Int16(1), aCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong COLUMN reference position after setting it", sal_Int32(2), aCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ROW reference position after setting it", sal_Int32(3), aCellAddress.Row);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
