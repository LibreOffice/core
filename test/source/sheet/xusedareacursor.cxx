/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xusedareacursor.hxx>

#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>

#include <com/sun/star/table/CellRangeAddress.hpp>

#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star;
using namespace com::sun::star::uno;


namespace apitest {

void XUsedAreaCursor::testGotoStartOfUsedArea()
{
    uno::Reference< sheet::XSpreadsheet > xSheet(getXSpreadsheet(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor(xSheet->createCursor(), UNO_QUERY_THROW);
    uno::Reference< sheet::XCellRangeAddressable> xCellRangeAddressable(xSheetCellCursor, UNO_QUERY_THROW);

    uno::Reference< sheet::XUsedAreaCursor > xUsedAreaCursor(xSheetCellCursor, UNO_QUERY_THROW);

    xUsedAreaCursor->gotoStartOfUsedArea(false);
    xUsedAreaCursor->gotoEndOfUsedArea(true);
    table::CellRangeAddress cellRangeAddress = xCellRangeAddressable->getRangeAddress();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start column",
                                 sal_Int32(0), cellRangeAddress.StartColumn);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end column",
                                 sal_Int32(2), cellRangeAddress.EndColumn);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start row",
                                 sal_Int32(0), cellRangeAddress.StartRow);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end column",
                                 sal_Int32(0), cellRangeAddress.EndRow);

    xUsedAreaCursor->gotoEndOfUsedArea(false);
    cellRangeAddress = xCellRangeAddressable->getRangeAddress();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start column",
                                 sal_Int32(2), cellRangeAddress.StartColumn);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end column",
                                 sal_Int32(2), cellRangeAddress.EndColumn);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start row",
                                 sal_Int32(0), cellRangeAddress.StartRow);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end column",
                                 sal_Int32(0), cellRangeAddress.EndRow);
}

void XUsedAreaCursor::testGotoEndOfUsedArea()
{
    uno::Reference< sheet::XSpreadsheet > xSheet(getXSpreadsheet(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor(xSheet->createCursor(), UNO_QUERY_THROW);
    uno::Reference< sheet::XCellRangeAddressable> xCellRangeAddressable(xSheetCellCursor, UNO_QUERY_THROW);

    uno::Reference< sheet::XUsedAreaCursor > xUsedAreaCursor(xSheetCellCursor, UNO_QUERY_THROW);

    xUsedAreaCursor->gotoEndOfUsedArea(false);
    xUsedAreaCursor->gotoStartOfUsedArea(true);
    table::CellRangeAddress cellRangeAddress = xCellRangeAddressable->getRangeAddress();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start column",
                                 sal_Int32(0), cellRangeAddress.StartColumn);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end column",
                                 sal_Int32(2), cellRangeAddress.EndColumn);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start row",
                                 sal_Int32(0), cellRangeAddress.StartRow);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end column",
                                 sal_Int32(0), cellRangeAddress.EndRow);

    xUsedAreaCursor->gotoStartOfUsedArea(false);
    cellRangeAddress = xCellRangeAddressable->getRangeAddress();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start column",
                                 sal_Int32(0), cellRangeAddress.StartColumn);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end column",
                                 sal_Int32(0), cellRangeAddress.EndColumn);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start row",
                                 sal_Int32(0), cellRangeAddress.StartRow);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end column",
                                 sal_Int32(0), cellRangeAddress.EndRow);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
