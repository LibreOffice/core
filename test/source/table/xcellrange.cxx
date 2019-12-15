/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/table/xcellrange.hxx>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;

namespace apitest
{
void XCellRange::testGetCellByPosition()
{
    uno::Reference<table::XCellRange> xCR(init(), uno::UNO_QUERY_THROW);

    uno::Reference<table::XCell> xCell(xCR->getCellByPosition(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xCell.is());

    CPPUNIT_ASSERT_THROW(xCR->getCellByPosition(-1, 1), lang::IndexOutOfBoundsException);
}

void XCellRange::testGetCellRangeByName()
{
    uno::Reference<table::XCellRange> xCR(init(), uno::UNO_QUERY_THROW);

    uno::Reference<table::XCellRange> xCellRange(xCR->getCellRangeByName(m_aRangeName),
                                                 uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xCellRange.is());
}

void XCellRange::testGetCellRangeByPosition()
{
    uno::Reference<table::XCellRange> xCR(init(), uno::UNO_QUERY_THROW);

    uno::Reference<table::XCellRange> xCellRange(xCR->getCellRangeByPosition(0, 0, 0, 0),
                                                 uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xCellRange.is());

    CPPUNIT_ASSERT_THROW(xCR->getCellRangeByPosition(-1, 0, -1, 1),
                         lang::IndexOutOfBoundsException);
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
