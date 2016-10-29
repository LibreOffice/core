/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xlabelrange.hxx>
#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include "cppunit/TestAssert.h"

using namespace css;
using namespace css::uno;

namespace apitest {

void XLabelRange::testDataArea()
{
    uno::Reference < sheet::XLabelRange > xLabelRange(init(),UNO_QUERY_THROW);
    table::CellRangeAddress gA = xLabelRange->getDataArea();
    table::CellRangeAddress setDAddress;
    setDAddress.Sheet = 0;
    setDAddress.StartRow = 1;
    setDAddress.EndRow = 2;
    setDAddress.StartColumn = 3;
    setDAddress.EndColumn = 4;

    CPPUNIT_ASSERT_MESSAGE("Wrong SHEET reference position", setDAddress.Sheet == gA.Sheet);
    CPPUNIT_ASSERT_MESSAGE("Wrong STARTROW reference position", setDAddress.StartRow == gA.StartRow);
    CPPUNIT_ASSERT_MESSAGE("Wrong ENDROW reference position", setDAddress.EndRow == gA.EndRow);
    CPPUNIT_ASSERT_MESSAGE("Wrong STARTCOLUMN reference position", setDAddress.StartColumn == gA.StartColumn);
    CPPUNIT_ASSERT_MESSAGE("Wrong ENDCOLUMN reference position", setDAddress.EndColumn == gA.EndColumn);

}

void XLabelRange::testLabelArea()
{
    uno::Reference < sheet::XLabelRange > xLabelRange(init(),UNO_QUERY_THROW);
    table::CellRangeAddress gA1 = xLabelRange->getLabelArea();
    table::CellRangeAddress setLAddress;
    setLAddress.Sheet = 0;
    setLAddress.StartRow = 1;
    setLAddress.EndRow = 2;
    setLAddress.StartColumn = 3;
    setLAddress.EndColumn = 4;

    CPPUNIT_ASSERT_MESSAGE("Wrong SHEET reference position", setLAddress.Sheet == gA1.Sheet);
    CPPUNIT_ASSERT_MESSAGE("Wrong STARTROW reference position", setLAddress.StartRow == gA1.StartRow);
    CPPUNIT_ASSERT_MESSAGE("Wrong ENDROW reference position", setLAddress.EndRow == gA1.EndRow);
    CPPUNIT_ASSERT_MESSAGE("Wrong STARTCOLUMN reference position", setLAddress.StartColumn == gA1.StartColumn);
    CPPUNIT_ASSERT_MESSAGE("Wrong ENDCOLUMN reference position", setLAddress.EndColumn == gA1.EndColumn);

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
