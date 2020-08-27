/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetoutline.hxx>

#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/table/TableOrientation.hpp>

#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/sheet/XSheetOutline.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>

#include <cppunit/TestAssert.h>
#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

const OUStringLiteral colLevel1 = u"OutlineSheet.A1:Z1";
const OUStringLiteral colLevel2 = u"OutlineSheet.C1:W1";
const OUStringLiteral colLevel3 = u"OutlineSheet.E1:U1";
const OUStringLiteral colLevel4 = u"OutlineSheet.G1:S1";

const OUStringLiteral rowLevel1 = u"OutlineSheet.A1:A30";
const OUStringLiteral rowLevel2 = u"OutlineSheet.A3:A27";

static OUString getVisibleAdress(
    uno::Reference<sheet::XSpreadsheet> const& xSheet,
    const OUString& aLevelRangeString)
{

    uno::Reference<table::XCellRange> aSheetRangeAccess(xSheet, UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> aLevelRange = aSheetRangeAccess->getCellRangeByName(aLevelRangeString);

    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(aLevelRange,UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryVisibleCells();
    OUString aResult = xRanges->getRangeAddressesAsString();

    return aResult;

}

static table::CellRangeAddress getAddressFromRangeString(
    uno::Reference<sheet::XSpreadsheet> const& xSheet,
    const OUString& aStringAddress)
{

    uno::Reference< table::XCellRange > aSheetRangeAccess(xSheet, UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> aRange = aSheetRangeAccess->getCellRangeByName(aStringAddress);

    uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddressable(aRange, UNO_QUERY_THROW);
    table::CellRangeAddress aLevelRangeAddress = xCellRangeAddressable->getRangeAddress();

    return aLevelRangeAddress;

}

void XSheetOutline::testHideDetail()
{

    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    // Column level 2
    table::CellRangeAddress aLevelRangeAddress = getAddressFromRangeString(aSheet, colLevel2);
    aSheetOutline->hideDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Column level 2 not hidden",
            getVisibleAdress(aSheet, colLevel2), OUString());

    // Row Level 2
    aLevelRangeAddress = getAddressFromRangeString(aSheet, rowLevel2);
    aSheetOutline->hideDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row level 2 not hidden",
            getVisibleAdress(aSheet, rowLevel2), OUString());

}

void XSheetOutline::testShowDetail()
{
    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    aSheetOutline->showDetail(getAddressFromRangeString(aSheet, colLevel1));
    aSheetOutline->showDetail(getAddressFromRangeString(aSheet, rowLevel1));

    // Row Level 2
    table::CellRangeAddress  aLevelRangeAddress = getAddressFromRangeString(aSheet, rowLevel2);
    aSheetOutline->showDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row level 2 still hidden",
            getVisibleAdress(aSheet, rowLevel2), OUString(rowLevel2));

    // Column Level 2
    aLevelRangeAddress = getAddressFromRangeString(aSheet, colLevel2);
    aSheetOutline->showDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Column level 2 still hidden",
            getVisibleAdress(aSheet, colLevel2), OUString(colLevel2));

}

void XSheetOutline::testShowLevel()
{

    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    // test columns

    table::CellRangeAddress aLevel1 = getAddressFromRangeString(aSheet, colLevel1);
    table::CellRangeAddress aLevel2 = getAddressFromRangeString(aSheet, colLevel2);
    table::CellRangeAddress aLevel3 = getAddressFromRangeString(aSheet, colLevel3);
    table::CellRangeAddress aLevel4 = getAddressFromRangeString(aSheet, colLevel4);

    aSheetOutline->showDetail(getAddressFromRangeString(aSheet, colLevel1));
    aSheetOutline->showDetail(getAddressFromRangeString(aSheet, colLevel2));
    aSheetOutline->showDetail(getAddressFromRangeString(aSheet, rowLevel1));

    aSheetOutline->hideDetail(aLevel1);
    aSheetOutline->hideDetail(aLevel2);
    aSheetOutline->hideDetail(aLevel3);
    aSheetOutline->hideDetail(aLevel4);

    aSheetOutline->showLevel(2, table::TableOrientation_COLUMNS);

    std::cout << " verify showLevel col apres" << getVisibleAdress(aSheet, "OutlineSheet.A1:Z30") << std::endl;

    // verify that level 2 and level 1 are shown --> column 0..3 & column 22..26
    // level 3 & 4 are hidden --> column 4..19

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Column", OUString("OutlineSheet.A1:D1"), getVisibleAdress(aSheet, "OutlineSheet.A1:D1"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Column", OUString("OutlineSheet.V1:Z1"), getVisibleAdress(aSheet, "OutlineSheet.V1:Z1"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Column", OUString(), getVisibleAdress(aSheet, colLevel3));

/* FIXME !!

    // test Rows

    aLevel1 = getAddressFromRangeString(rowLevel1);
    aLevel2 = getAddressFromRangeString(rowLevel2);
    aLevel3 = getAddressFromRangeString(rowLevel3);
    aLevel4 = getAddressFromRangeString(rowLevel4);

    aSheetOutline->showDetail(getAddressFromRangeString(colLevel1));
    aSheetOutline->showDetail(getAddressFromRangeString(rowLevel1));
    aSheetOutline->showDetail(getAddressFromRangeString(rowLevel2));

    std::cout << " verify showLevel row before" << OUStringToOString(getVisibleAdress("OutlineSheet.A1:Z30"), RTL_TEXTENCODING_UTF8).getStr() << std::endl;

    aSheetOutline->hideDetail(aLevel1);
    aSheetOutline->hideDetail(aLevel2);
    aSheetOutline->hideDetail(aLevel3);
    aSheetOutline->hideDetail(aLevel4);

    std::cout << " verify showLevel row just before" << OUStringToOString(getVisibleAdress("OutlineSheet.A1:Z30"), RTL_TEXTENCODING_UTF8).getStr() << std::endl;

    aSheetOutline->showLevel(2, table::TableOrientation_ROWS);

    std::cout << " verify showLevel row after" << OUStringToOString(getVisibleAdress("OutlineSheet.A1:Z30"), RTL_TEXTENCODING_UTF8).getStr() << std::endl;

    // verify rows visible 0..3 & 24..29
    // verify rows hidden 4..23

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Row", OUString("OutlineSheet.A1:A4"), getVisibleAdress("OutlineSheet.A1:A4"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Row", OUString("OutlineSheet.A25:A30"), getVisibleAdress("OutlineSheet.A25:A30"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Row", OUString(), getVisibleAdress(rowLevel3));

*/

}

void XSheetOutline::testGroup()
{

    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    // Column level 2
    OUString aNewString = "OutlineSheet.AB1:AG1";
    table::CellRangeAddress aLevelRangeAddress = getAddressFromRangeString(aSheet, aNewString);
    aSheetOutline->group(aLevelRangeAddress, table::TableOrientation_COLUMNS);

    aSheetOutline->hideDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testGroup Column still visible", getVisibleAdress(aSheet, aNewString), OUString());

    // Row Level 2
    aNewString = "OutlineSheet.A50:A60";
    aLevelRangeAddress = getAddressFromRangeString(aSheet, aNewString);
    aSheetOutline->group(aLevelRangeAddress, table::TableOrientation_ROWS);

    aSheetOutline->hideDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testGroup Row still visible", getVisibleAdress(aSheet, aNewString), OUString());

}

void XSheetOutline::testUngroup()
{
    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    // New Column level
    OUString aNewString = "OutlineSheet.BB1:BG1";
    table::CellRangeAddress aLevelRangeAddress = getAddressFromRangeString(aSheet, aNewString);
    aSheetOutline->group(aLevelRangeAddress, table::TableOrientation_COLUMNS);
    aSheetOutline->hideDetail(aLevelRangeAddress);

    aSheetOutline->ungroup(aLevelRangeAddress, table::TableOrientation_COLUMNS);

    aSheetOutline->showDetail(aLevelRangeAddress);
    // should remain hidden ?

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testUnGroup Column becomes visible after ungroup", getVisibleAdress(aSheet, aNewString), OUString());

    // New Row level
    aNewString = "OutlineSheet.A70:A80";
    aLevelRangeAddress = getAddressFromRangeString(aSheet, aNewString);
    aSheetOutline->group(aLevelRangeAddress, table::TableOrientation_ROWS);
    aSheetOutline->hideDetail(aLevelRangeAddress);

    aSheetOutline->ungroup(aLevelRangeAddress, table::TableOrientation_ROWS);

    aSheetOutline->showDetail(aLevelRangeAddress);
    // should remain hidden ?

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testUnGroup Row becomes visible after ungroup", getVisibleAdress(aSheet, aNewString), OUString());

}

void XSheetOutline::testClearOutline()
{
    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    aSheetOutline->clearOutline();

    table::CellRangeAddress aLevelRangeAddress = getAddressFromRangeString(aSheet, colLevel1);
    aSheetOutline->hideDetail(aLevelRangeAddress);
    aLevelRangeAddress = getAddressFromRangeString(aSheet, rowLevel1);
    aSheetOutline->hideDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testClearOutline Columns are hidden after clear", getVisibleAdress(aSheet, colLevel1), OUString(colLevel1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testClearOutline Rows are hidden after clear", getVisibleAdress(aSheet, rowLevel1), OUString(rowLevel1));

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
