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
#include <com/sun/star/sheet/XCellRangesQuery.hpp>

#include "cppunit/extensions/HelperMacros.h"
#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

const OUString colLevel1 = "OutlineSheet.A1:Z1";
const OUString colLevel2 = "OutlineSheet.C1:W1";
const OUString colLevel3 = "OutlineSheet.E1:U1";
const OUString colLevel4 = "OutlineSheet.G1:S1";

const OUString rowLevel1 = "OutlineSheet.A1:A30";
const OUString rowLevel2 = "OutlineSheet.A3:A27";
const OUString rowLevel3 = "OutlineSheet.A5:A24";
const OUString rowLevel4 = "OutlineSheet.A7:A21";

uno::Reference< sheet::XSpreadsheet > aSheetGlobal;

OUString getVisibleAdress(const OUString& aLevelRangeString)
{

    uno::Reference<table::XCellRange> aSheetRangeAccess(aSheetGlobal, UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> aLevelRange = aSheetRangeAccess->getCellRangeByName(aLevelRangeString);

    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(aLevelRange,UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryVisibleCells();
    OUString aResult = xRanges->getRangeAddressesAsString();

    return aResult;

}

table::CellRangeAddress getAddressFromRangeString(const OUString& aStringAddress)
{

    uno::Reference< table::XCellRange > aSheetRangeAccess(aSheetGlobal, UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> aRange = aSheetRangeAccess->getCellRangeByName(aStringAddress);

    uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddressable(aRange, UNO_QUERY_THROW);
    table::CellRangeAddress aLevelRangeAddress = xCellRangeAddressable->getRangeAddress();

    return aLevelRangeAddress;

}

void XSheetOutline::testHideDetail()
{

    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    aSheetGlobal = aSheet;

    // Column level 2
    table::CellRangeAddress aLevelRangeAddress = getAddressFromRangeString(colLevel2);
    aSheetOutline->hideDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Column level 2 not hidden", getVisibleAdress(colLevel2), OUString(""));

    // Row Level 2
    aLevelRangeAddress = getAddressFromRangeString(rowLevel2);
    aSheetOutline->hideDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row level 2 not hidden", getVisibleAdress(rowLevel2), OUString(""));

}

void XSheetOutline::testShowDetail()
{
    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    aSheetGlobal = aSheet;
    aSheetOutline->showDetail(getAddressFromRangeString(colLevel1));
    aSheetOutline->showDetail(getAddressFromRangeString(rowLevel1));

    // Row Level 2
    table::CellRangeAddress  aLevelRangeAddress = getAddressFromRangeString(rowLevel2);
    aSheetOutline->showDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row level 2 still hidden", getVisibleAdress(rowLevel2), rowLevel2);

    // Column Level 2
    aLevelRangeAddress = getAddressFromRangeString(colLevel2);
    aSheetOutline->showDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Column level 2 still hidden", getVisibleAdress(colLevel2), colLevel2);

}

void XSheetOutline::testShowLevel()
{

    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    aSheetGlobal = aSheet;

    table::CellRangeAddress aLevelRangeAddress;

    // test columns

    table::CellRangeAddress aLevel1 = getAddressFromRangeString(colLevel1);
    table::CellRangeAddress aLevel2 = getAddressFromRangeString(colLevel2);
    table::CellRangeAddress aLevel3 = getAddressFromRangeString(colLevel3);
    table::CellRangeAddress aLevel4 = getAddressFromRangeString(colLevel4);

    aSheetOutline->showDetail(getAddressFromRangeString(colLevel1));
    aSheetOutline->showDetail(getAddressFromRangeString(colLevel2));
    aSheetOutline->showDetail(getAddressFromRangeString(rowLevel1));

    aSheetOutline->hideDetail(aLevel1);
    aSheetOutline->hideDetail(aLevel2);
    aSheetOutline->hideDetail(aLevel3);
    aSheetOutline->hideDetail(aLevel4);

    aSheetOutline->showLevel(2, table::TableOrientation_COLUMNS);

    std::cout << " verifiy showLevel col apres" << OUStringToOString(getVisibleAdress("OutlineSheet.A1:Z30"), RTL_TEXTENCODING_UTF8).getStr() << std::endl;

    // verify that level 2 and level 1 are shown --> column 0..3 & column 22..26
    // level 3 & 4 are hidden --> column 4..19

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Column", OUString("OutlineSheet.A1:D1"), getVisibleAdress("OutlineSheet.A1:D1"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Column", OUString("OutlineSheet.V1:Z1"), getVisibleAdress("OutlineSheet.V1:Z1"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Column", OUString(""), getVisibleAdress(colLevel3));

/* FIXME !!

    // test Rows

    aLevel1 = getAddressFromRangeString(rowLevel1);
    aLevel2 = getAddressFromRangeString(rowLevel2);
    aLevel3 = getAddressFromRangeString(rowLevel3);
    aLevel4 = getAddressFromRangeString(rowLevel4);

    aSheetOutline->showDetail(getAddressFromRangeString(colLevel1));
    aSheetOutline->showDetail(getAddressFromRangeString(rowLevel1));
    aSheetOutline->showDetail(getAddressFromRangeString(rowLevel2));

    std::cout << " verifiy showLevel row avant" << OUStringToOString(getVisibleAdress("OutlineSheet.A1:Z30"), RTL_TEXTENCODING_UTF8).getStr() << std::endl;

    aSheetOutline->hideDetail(aLevel1);
    aSheetOutline->hideDetail(aLevel2);
    aSheetOutline->hideDetail(aLevel3);
    aSheetOutline->hideDetail(aLevel4);

    std::cout << " verifiy showLevel row juste avant" << OUStringToOString(getVisibleAdress("OutlineSheet.A1:Z30"), RTL_TEXTENCODING_UTF8).getStr() << std::endl;

    aSheetOutline->showLevel(2, table::TableOrientation_ROWS);

    std::cout << " verifiy showLevel row apres" << OUStringToOString(getVisibleAdress("OutlineSheet.A1:Z30"), RTL_TEXTENCODING_UTF8).getStr() << std::endl;

    // verify rows visible 0..3 & 24..29
    // verfiy rows hidden 4..23

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Row", OUString("OutlineSheet.A1:A4"), getVisibleAdress("OutlineSheet.A1:A4"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Row", OUString("OutlineSheet.A25:A30"), getVisibleAdress("OutlineSheet.A25:A30"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testShowLevel Row", OUString(""), getVisibleAdress(rowLevel3));

*/

}

void XSheetOutline::testGroup()
{

    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    aSheetGlobal = aSheet;

    // Column level 2
    OUString aNewString = "OutlineSheet.AB1:AG1";
    table::CellRangeAddress aLevelRangeAddress = getAddressFromRangeString(aNewString);
    aSheetOutline->group(aLevelRangeAddress, table::TableOrientation_COLUMNS);

    aSheetOutline->hideDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testGroup Column still visible", getVisibleAdress(aNewString), OUString(""));

    // Row Level 2
    aNewString = "OutlineSheet.A50:A60";
    aLevelRangeAddress = getAddressFromRangeString(aNewString);
    aSheetOutline->group(aLevelRangeAddress, table::TableOrientation_ROWS);

    aSheetOutline->hideDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testGroup Row still visible", getVisibleAdress(aNewString), OUString(""));

}

void XSheetOutline::testUngroup()
{
    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    aSheetGlobal = aSheet;

    // New Column level
    OUString aNewString = "OutlineSheet.BB1:BG1";
    table::CellRangeAddress aLevelRangeAddress = getAddressFromRangeString(aNewString);
    aSheetOutline->group(aLevelRangeAddress, table::TableOrientation_COLUMNS);
    aSheetOutline->hideDetail(aLevelRangeAddress);

    aSheetOutline->ungroup(aLevelRangeAddress, table::TableOrientation_COLUMNS);

    aSheetOutline->showDetail(aLevelRangeAddress);
    // should remain hidden ?

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testUnGroup Column becomes visible after ungroup", getVisibleAdress(aNewString), OUString(""));

    // New Row level
    aNewString = "OutlineSheet.A70:A80";
    aLevelRangeAddress = getAddressFromRangeString(aNewString);
    aSheetOutline->group(aLevelRangeAddress, table::TableOrientation_ROWS);
    aSheetOutline->hideDetail(aLevelRangeAddress);

    aSheetOutline->ungroup(aLevelRangeAddress, table::TableOrientation_ROWS);

    aSheetOutline->showDetail(aLevelRangeAddress);
    // should remain hidden ?

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testUnGroup Row becomes visible after ungroup", getVisibleAdress(aNewString), OUString(""));

}

void XSheetOutline::testAutoOutline()
{
}

void XSheetOutline::testClearOutline()
{
    uno::Reference< sheet::XSpreadsheet > aSheet(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetOutline > aSheetOutline(aSheet, UNO_QUERY_THROW);

    aSheetGlobal = aSheet;

    aSheetOutline->clearOutline();

    table::CellRangeAddress aLevelRangeAddress = getAddressFromRangeString(colLevel1);
    aSheetOutline->hideDetail(aLevelRangeAddress);
    aLevelRangeAddress = getAddressFromRangeString(rowLevel1);
    aSheetOutline->hideDetail(aLevelRangeAddress);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("testClearOutline Columns are hidden after clear", getVisibleAdress(colLevel1), colLevel1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testClearOutline Rows are hidden after clear", getVisibleAdress(rowLevel1), rowLevel1);

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
