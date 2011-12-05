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
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
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
#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellRange.hpp>

namespace ScCellRangeBase {

class ScCellProperties : public UnoApiTest
{
    uno::Reference < beans::XPropertySet > init();

    void testVertJustify();
    void testRotateReference();
    CPPUNIT_TEST_SUITE(ScCellProperties);
    CPPUNIT_TEST(testVertJustify);
    CPPUNIT_TEST(testRotateReference);
    CPPUNIT_TEST_SUITE_END();
};

void ScCellProperties::testVertJustify()
{
    uno::Reference< beans::XPropertySet > xCellRangeBase = init();
    rtl::OUString aVertJustify(RTL_CONSTASCII_USTRINGPARAM("VertJustify"));
    uno::Any aOldVertJustify = xCellRangeBase->getPropertyValue(aVertJustify);
    sal_Int32 aValue;
    CPPUNIT_ASSERT(aOldVertJustify >>= aValue);
    std::cout << "Old VertJustify value: " << aValue << std::endl;

    uno::Any aNewVertJustify;
    aNewVertJustify <<= static_cast<sal_Int32>(3);
    xCellRangeBase->setPropertyValue(aVertJustify, aNewVertJustify);
    uno::Any aVertJustifyControllValue = xCellRangeBase->getPropertyValue(aVertJustify);
    CPPUNIT_ASSERT(aVertJustifyControllValue >>= aValue);
    std::cout << "New VertJustify value: " << aValue << std::endl;
    CPPUNIT_ASSERT_MESSAGE("value has not been changed", aValue == 3);
}

void ScCellProperties::testRotateReference()
{
    uno::Reference< beans::XPropertySet > xCellRangeBase = init();
    rtl::OUString aRotateReference(RTL_CONSTASCII_USTRINGPARAM("RotateReference"));
    uno::Any aOldRotateReference = xCellRangeBase->getPropertyValue(aRotateReference);
    sal_Int32 aValue;
    CPPUNIT_ASSERT(aOldRotateReference >>= aValue);
    std::cout << "Old RotateReference Value: " << aValue << std::endl;

    uno::Any aNewRotateReference;
    aNewRotateReference <<= static_cast<sal_Int32>(3);
    xCellRangeBase->setPropertyValue(aRotateReference, aNewRotateReference);
    uno::Any aRotateReferenceControllValue = xCellRangeBase->getPropertyValue(aRotateReference);
    CPPUNIT_ASSERT(aRotateReferenceControllValue >>= aValue);
    std::cout << "New RotateReference value: " << aValue << std::endl;
    CPPUNIT_ASSERT_MESSAGE("value has not been changed", aValue == 3);
}

uno::Reference< beans::XPropertySet > ScCellProperties::init()
{
    rtl::OUString aFileURL;
    const rtl::OUString aFileBase(RTL_CONSTASCII_USTRINGPARAM("xcellrangesquery.ods"));
    createFileURL(aFileBase, aFileURL);
    static uno::Reference< lang::XComponent > xComponent;
    if( !xComponent.is())
        xComponent = loadFromDesktop(aFileURL);
    uno::Reference< sheet::XSpreadsheetDocument> xDoc (xComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());

    uno::Reference< table::XCellRange > xCellRange(xSheet->getCellRangeByPosition(0,0,3,4), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xCellRange.is());

    uno::Reference< beans::XPropertySet > xReturn(xCellRange, UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create object of type XPropertySet", xReturn.is());
    return xReturn;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCellProperties);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
