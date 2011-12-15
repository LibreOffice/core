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

#include <test/unoapi_test.hxx>

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

#include <rtl/oustringostreaminserter.hxx>

namespace ScNamedRangeObj {

class ScXNamedRange : public UnoApiTest
{
    uno::Reference< sheet::XSpreadsheetDocument> init();
    uno::Reference< sheet::XNamedRange> getTestedNamedRange(const rtl::OUString&);

    // XNamedRange
    void testGetContent();
    void testSetContent();
    void testGetType();
    void testSetType();
    void testGetReferencePosition();
    void testSetReferencePosition();

    CPPUNIT_TEST_SUITE(ScXNamedRange);
    CPPUNIT_TEST(testGetContent);
    CPPUNIT_TEST(testSetContent);
    CPPUNIT_TEST(testGetType);
    CPPUNIT_TEST(testSetType);
    CPPUNIT_TEST(testGetReferencePosition);
    CPPUNIT_TEST(testSetReferencePosition);
    CPPUNIT_TEST_SUITE_END();
};


void ScXNamedRange::testGetContent()
{
    rtl::OUString aTestedNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("initial1"));
    uno::Reference< sheet::XNamedRange > xNamedRange = getTestedNamedRange(aTestedNamedRangeString);

    rtl::OUString aExpectedContent(RTL_CONSTASCII_USTRINGPARAM("$Sheet1.$B$1"));
    CPPUNIT_ASSERT_MESSAGE("Wrong expected content for initial1 on GetContent", xNamedRange->getContent().equals(aExpectedContent));
}

void ScXNamedRange::testSetContent()
{
    rtl::OUString aTestedNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("initial1"));
    uno::Reference< sheet::XNamedRange > xNamedRange = getTestedNamedRange(aTestedNamedRangeString);

    rtl::OUString aExpectedContent;

    // test a cell
    aExpectedContent = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("D1"));
    xNamedRange->setContent(aExpectedContent);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected content for initial1 after SetContent a cell", xNamedRange->getContent().equals(aExpectedContent));

    // test a cellrange
    aExpectedContent = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("D1:D10"));
    xNamedRange->setContent(aExpectedContent);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected content for initial1 after SetContent a cellrange", xNamedRange->getContent().equals(aExpectedContent));

    // test a formula
    aExpectedContent = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("=D10"));
    xNamedRange->setContent(aExpectedContent);
    aExpectedContent = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("D10"));
    CPPUNIT_ASSERT_MESSAGE("Wrong expected content for initial1 after SetContent a formula", xNamedRange->getContent().equals(aExpectedContent));

}

void ScXNamedRange::testGetType()
{
    rtl::OUString aTestedNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("initial1"));
    uno::Reference< sheet::XNamedRange > xNamedRange = getTestedNamedRange(aTestedNamedRangeString);
    CPPUNIT_ASSERT_MESSAGE("Wrong expected Type", xNamedRange->getType() == 0);
}

void ScXNamedRange::testSetType()
{
    rtl::OUString aTestedNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("initial1"));
    uno::Reference< sheet::XNamedRange > xNamedRange = getTestedNamedRange(aTestedNamedRangeString);

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

void ScXNamedRange::testGetReferencePosition()
{
    rtl::OUString aTestedNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("initial2"));
    uno::Reference< sheet::XNamedRange > xNamedRange = getTestedNamedRange(aTestedNamedRangeString);

    table::CellAddress xCellAddress = xNamedRange->getReferencePosition();
    // the expeted address is on B1, as it was the active cell when intial2 created
    CPPUNIT_ASSERT_MESSAGE("Wrong SHEET reference position", xCellAddress.Sheet == 0);
    CPPUNIT_ASSERT_MESSAGE("Wrong COLUMN reference position", xCellAddress.Column == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong ROW reference position", xCellAddress.Row == 0);
}

void ScXNamedRange::testSetReferencePosition()
{
    rtl::OUString aTestedNamedRangeString(RTL_CONSTASCII_USTRINGPARAM("initial1"));
    uno::Reference< sheet::XNamedRange > xNamedRange = getTestedNamedRange(aTestedNamedRangeString);

    table::CellAddress aBaseAddress = table::CellAddress(1,2,3);

    xNamedRange->setReferencePosition(aBaseAddress);

    table::CellAddress xCellAddress = xNamedRange->getReferencePosition();
    CPPUNIT_ASSERT_MESSAGE("Wrong SHEET reference position after setting it", xCellAddress.Sheet == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong COLUMN reference position after setting it", xCellAddress.Column == 2);
    CPPUNIT_ASSERT_MESSAGE("Wrong ROW reference position after setting it", xCellAddress.Row == 3);
}

uno::Reference< sheet::XSpreadsheetDocument> ScXNamedRange::init()
{
    rtl::OUString aFileURL;
    const rtl::OUString aFileBase(RTL_CONSTASCII_USTRINGPARAM("rangenames.ods"));
    createFileURL(aFileBase, aFileURL);
    static uno::Reference< lang::XComponent > xComponent;
    if( !xComponent.is())
        xComponent = loadFromDesktop(aFileURL);
    uno::Reference< sheet::XSpreadsheetDocument> xDoc (xComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDoc.is());

    return xDoc;
}

uno::Reference< sheet::XNamedRange> ScXNamedRange::getTestedNamedRange(const rtl::OUString& aTestedNamedRangeString)
{
    uno::Reference< sheet::XSpreadsheetDocument> xDoc = init();
    uno::Reference< beans::XPropertySet > xPropSet (xDoc, UNO_QUERY_THROW);
    rtl::OUString aNamedRangesPropertyString(RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
    uno::Reference< container::XNameAccess > xNamedRangesNameAccess(xPropSet->getPropertyValue(aNamedRangesPropertyString), UNO_QUERY_THROW);

    uno::Reference< sheet::XNamedRange > xNamedRange(xNamedRangesNameAccess->getByName(aTestedNamedRangeString), UNO_QUERY_THROW);

    return xNamedRange;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScXNamedRange);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
