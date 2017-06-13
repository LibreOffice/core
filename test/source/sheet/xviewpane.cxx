/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xviewpane.hxx>
#include <com/sun/star/sheet/XViewPane.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include "cppunit/TestAssert.h"

using namespace css;
using namespace css::uno;

namespace apitest {

void XViewPane::testFirstVisibleColumn()
{
    sal_Int32 nCol = 5;
    uno::Reference < sheet::XViewPane > xViewPane(init(),UNO_QUERY_THROW);
    xViewPane->setFirstVisibleColumn(nCol);
    CPPUNIT_ASSERT_EQUAL(xViewPane->getFirstVisibleColumn(), nCol);
}

void XViewPane::testFirstVisibleRow()
{
    sal_Int32 nRow = 3;
    uno::Reference < sheet::XViewPane > xViewPane(init(),UNO_QUERY_THROW);
    xViewPane->setFirstVisibleRow(nRow);
    CPPUNIT_ASSERT_EQUAL(xViewPane->getFirstVisibleRow(), nRow);
}

void XViewPane::testVisibleRange()
{
    sal_Int32 nCol = 5;
    sal_Int32 nRow = 3;
    uno::Reference < sheet::XViewPane > xViewPane(init(),UNO_QUERY_THROW);
    xViewPane->setFirstVisibleColumn(nCol);
    xViewPane->setFirstVisibleRow(nRow);

    table::CellRangeAddress aCellRangeAddress = xViewPane->getVisibleRange();
    CPPUNIT_ASSERT_EQUAL(aCellRangeAddress.Sheet, short(0));
    CPPUNIT_ASSERT_EQUAL(aCellRangeAddress.StartRow, nRow);
    CPPUNIT_ASSERT_EQUAL(aCellRangeAddress.StartColumn, nCol);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
