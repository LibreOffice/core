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
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/Border.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>

#include <rtl/oustringostreaminserter.hxx>

namespace ScNamedRangeObj {

class ScXNamedRange : public UnoApiTest
{
    uno::Reference< sheet::XSpreadsheetDocument> init();

    // XNamedRanges
    void testAddNewByName();
    void testAddNewFromTitles();
    void testRemoveByName();
    void testOutputList();

    CPPUNIT_TEST_SUITE(ScXNamedRange);
    CPPUNIT_TEST(testAddNewByName);
    CPPUNIT_TEST(testAddNewFromTitles);
    //fix first warning in ExceptionsTestCaseDecorator
    //CPPUNIT_TEST_EXCEPTION(testRemoveByName, uno::RuntimeException);
    CPPUNIT_TEST(testOutputList);
    CPPUNIT_TEST_SUITE_END();
};


void ScXNamedRange::testAddNewByName()
{
    uno::Reference< sheet::XSpreadsheetDocument> xDoc = init();
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference< beans::XPropertySet > propSet (xDoc, UNO_QUERY_THROW);
    rtl::OUString namedRangesString(RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
    uno::Reference< sheet::XNamedRanges > xNamedRanges(propSet->getPropertyValue(namedRangesString), UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xNamedRangesNameAccess(propSet->getPropertyValue(namedRangesString), UNO_QUERY_THROW);

    table::CellAddress xBaseAddress = table::CellAddress(0,0,0);

    sal_Int32 nType = 0;
    rtl::OUString content1(RTL_CONSTASCII_USTRINGPARAM("D1"));
    rtl::OUString name1(RTL_CONSTASCII_USTRINGPARAM("type_0"));
    xNamedRanges->addNewByName(name1, content1, xBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type 0 - Normal case", xNamedRanges->hasByName(name1));

    nType = ::sheet::NamedRangeFlag::COLUMN_HEADER;
    rtl::OUString content2(RTL_CONSTASCII_USTRINGPARAM("D2"));
    rtl::OUString name2(RTL_CONSTASCII_USTRINGPARAM("type_COLUMN_HEADER"));
    xNamedRanges->addNewByName(name2, content2, xBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type COLUMN_HEADER", xNamedRanges->hasByName(name2));

    nType = ::sheet::NamedRangeFlag::FILTER_CRITERIA;
    rtl::OUString content3(RTL_CONSTASCII_USTRINGPARAM("D3"));
    rtl::OUString name3(RTL_CONSTASCII_USTRINGPARAM("type_FILTER_CRITERIA"));
    xNamedRanges->addNewByName(name3, content3, xBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type FILTER_CRITERIA", xNamedRanges->hasByName(name3));

    nType = ::sheet::NamedRangeFlag::PRINT_AREA;
    rtl::OUString content4(RTL_CONSTASCII_USTRINGPARAM("D4"));
    rtl::OUString name4(RTL_CONSTASCII_USTRINGPARAM("type_PRINT_AREA"));
    xNamedRanges->addNewByName(name4, content4, xBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type PRINT_AREA", xNamedRanges->hasByName(name4));

    nType = ::sheet::NamedRangeFlag::ROW_HEADER;
    rtl::OUString content5(RTL_CONSTASCII_USTRINGPARAM("D5"));
    rtl::OUString name5(RTL_CONSTASCII_USTRINGPARAM("type_ROW_HEADERA"));
    xNamedRanges->addNewByName(name5, content5, xBaseAddress, nType);
    CPPUNIT_ASSERT_MESSAGE("Failed to create Namedrange Type ROW_HEADER", xNamedRanges->hasByName(name5));

}

void ScXNamedRange::testAddNewFromTitles()
{

    uno::Reference< sheet::XSpreadsheetDocument> xDoc = init();
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(1), UNO_QUERY_THROW);

    uno::Reference< beans::XPropertySet > propSet (xDoc, UNO_QUERY_THROW);
    rtl::OUString namedRangesString(RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
    uno::Reference< sheet::XNamedRanges > xNamedRanges(propSet->getPropertyValue(namedRangesString), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xNamedRangesIndex(propSet->getPropertyValue(namedRangesString), UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xNamedRangesNameAccess(propSet->getPropertyValue(namedRangesString), UNO_QUERY_THROW);

    table::CellRangeAddress xCellRangeAddress = table::CellRangeAddress(1,0,0,3,3);

    rtl::OUString aString;
    uno::Reference< table::XCell > xCell;

    xNamedRanges->addNewFromTitles(xCellRangeAddress, sheet::Border_TOP);

    for (sal_Int32 i = 1; i < 4; i++)
    {
        // verify namedrange exists
        xCell = xSheet->getCellByPosition(i,0);
        uno::Reference< text::XTextRange > xTextRange(xCell, UNO_QUERY_THROW);
        aString = xTextRange->getString();
        std::cout << "verify " << aString << std::endl;
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

    xNamedRanges->addNewFromTitles(xCellRangeAddress, sheet::Border_LEFT);

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

void ScXNamedRange::testRemoveByName()
{
    uno::Reference< sheet::XSpreadsheetDocument> xDoc = init();
    uno::Reference< beans::XPropertySet > propSet (xDoc, UNO_QUERY_THROW);
    rtl::OUString namedRangesString(RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
    uno::Reference< sheet::XNamedRanges > xNamedRanges(propSet->getPropertyValue(namedRangesString), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex(propSet->getPropertyValue(namedRangesString), UNO_QUERY_THROW);

    rtl::OUString aNr1(RTL_CONSTASCII_USTRINGPARAM("initial1"));
    bool bHasIt = xNamedRanges->hasByName(aNr1);
    CPPUNIT_ASSERT_MESSAGE("NamedRange initial1 des not exits, can't remove it", bHasIt);

    if (bHasIt)
    {
        // remove existing
        sal_Int32 nInitialCount = xIndex->getCount();
        xNamedRanges->removeByName(aNr1);
        sal_Int32 nNewCount = xIndex->getCount();
        CPPUNIT_ASSERT_MESSAGE("NamedRange initial1 not removed", nNewCount == nInitialCount - 1);
        bool bHasItAgain = xNamedRanges->hasByName(aNr1);
        CPPUNIT_ASSERT_MESSAGE("Wrong NamedRange removed, initial1 still present", !bHasItAgain);

        rtl::OUString nr2(RTL_CONSTASCII_USTRINGPARAM("dummyNamedRange"));
        xNamedRanges->removeByName(aNr1);
        // should throw an exception
    }
}

void ScXNamedRange::testOutputList()
{

    uno::Reference< sheet::XSpreadsheetDocument> xDoc = init();
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    table::CellAddress xCellAddress = table::CellAddress (0,2,0);

    uno::Reference< beans::XPropertySet > propSet (xDoc, UNO_QUERY_THROW);
    rtl::OUString namedRangesString(RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
    uno::Reference< sheet::XNamedRanges > xNamedRanges(propSet->getPropertyValue(namedRangesString), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xNamedRangesIndex(propSet->getPropertyValue(namedRangesString), UNO_QUERY_THROW);
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

CPPUNIT_TEST_SUITE_REGISTRATION(ScXNamedRange);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
