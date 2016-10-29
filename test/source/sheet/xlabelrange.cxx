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
#include <thread>
#include "cppunit/TestAssert.h"
#include "cppunit/extensions/HelperMacros.h"

using namespace css;
using namespace css::uno;

namespace apitest {

void XLabelRange::testDataArea()
{
    unsigned int nr = std::thread::hardware_concurrency();
    table::CellRangeAddress setDAddress = table::CellRangeAddress(1, nr, 1, nr, 8);
    uno::Reference < sheet::XLabelRange > xLabelRange(init(),UNO_QUERY_THROW);
    table::CellRangeAddress gA = xLabelRange->getDataArea();
    CPPUNIT_ASSERT_EQUAL( setDAddress.Sheet, gA.Sheet );
    CPPUNIT_ASSERT_EQUAL( setDAddress.StartRow, gA.StartRow );
    CPPUNIT_ASSERT_EQUAL( setDAddress.EndRow, gA.EndRow );
    CPPUNIT_ASSERT_EQUAL( setDAddress.StartColumn, gA.StartColumn );
    CPPUNIT_ASSERT_EQUAL( setDAddress.EndColumn, gA.EndColumn );

}

void XLabelRange::testLabelArea()
{
    unsigned int nr = std::thread::hardware_concurrency();
    table::CellRangeAddress setLAddress = table::CellRangeAddress(1, nr, 0, nr, 0);
    uno::Reference < sheet::XLabelRange > xLabelRange(init(),UNO_QUERY_THROW);
    table::CellRangeAddress gA1 = xLabelRange->getLabelArea();
    CPPUNIT_ASSERT_EQUAL( setLAddress.Sheet, gA1.Sheet );
    CPPUNIT_ASSERT_EQUAL( setLAddress.StartRow, gA1.StartRow );
    CPPUNIT_ASSERT_EQUAL( setLAddress.EndRow, gA1.EndRow );
    CPPUNIT_ASSERT_EQUAL( setLAddress.StartColumn, gA1.StartColumn );
    CPPUNIT_ASSERT_EQUAL( setLAddress.EndColumn, gA1.EndColumn );

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
