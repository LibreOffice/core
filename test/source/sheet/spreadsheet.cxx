/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/spreadsheet.hxx>
#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XConditionalFormats.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void Spreadsheet::testSpreadsheetProperties()
{
    uno::Reference<beans::XPropertySet> xSpreadsheet(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "IsVisible";
    testBooleanProperty(xSpreadsheet, propName);

    propName = "PageStyle";
    testStringProperty(xSpreadsheet, propName, u"Report"_ustr);

    propName = "TableLayout";
    testShortProperty(xSpreadsheet, propName, text::WritingMode2::RL_TB);

    propName = "AutomaticPrintArea";
    testBooleanProperty(xSpreadsheet, propName);

    propName = "TabColor";
    util::Color aColorGet;
    util::Color aColorSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: TabColor",
                           xSpreadsheet->getPropertyValue(propName) >>= aColorGet);

    aNewValue <<= util::Color(42);
    xSpreadsheet->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheet->getPropertyValue(propName) >>= aColorSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue: TabColor", util::Color(42),
                                 aColorSet);

    propName = "ConditionalFormats";
    uno::Reference<sheet::XConditionalFormats> xConditionalFormatsGet;
    uno::Reference<sheet::XConditionalFormats> xConditionalFormatsSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: ConditionalFormats",
                           xSpreadsheet->getPropertyValue(propName) >>= xConditionalFormatsGet);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(getXSpreadsheetDocument(), UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, UNO_QUERY_THROW);
    uno::Reference<container::XNameContainer> xRanges(
        xMSF->createInstance(u"com.sun.star.sheet.SheetCellRanges"_ustr), UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xSheetCellRanges(xRanges, UNO_QUERY_THROW);

    uno::Any xCellRange;
    xCellRange <<= xSheet->getCellRangeByName(u"C1:D4"_ustr);
    xRanges->insertByName(u"Range1"_ustr, xCellRange);
    xConditionalFormatsGet->createByRange(xSheetCellRanges);

    aNewValue <<= xConditionalFormatsGet;
    xSpreadsheet->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheet->getPropertyValue(propName) >>= xConditionalFormatsSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue: ConditionalFormats", sal_Int32(1),
                                 xConditionalFormatsSet->getLength());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
