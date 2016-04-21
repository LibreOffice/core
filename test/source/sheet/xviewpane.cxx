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
    sal_Int32 col = 5;
    uno::Reference < sheet::XViewPane > xViewPane(init(),UNO_QUERY_THROW);
    xViewPane->setFirstVisibleColumn(col);
    bool bResult = col == xViewPane->getFirstVisibleColumn();
    CPPUNIT_ASSERT_MESSAGE("getFirstVisibleColumn()", bResult);
}

void XViewPane::testFirstVisibleRow()
{
    sal_Int32 row = 3;
    uno::Reference < sheet::XViewPane > xViewPane(init(),UNO_QUERY_THROW);
    xViewPane->setFirstVisibleRow(row);
    bool bResult = row == xViewPane->getFirstVisibleRow();
    CPPUNIT_ASSERT_MESSAGE("getFirstVisibleRow()", bResult);
}

void XViewPane::testVisibleRange()
{
    sal_Int32 col = 5;
    sal_Int32 row = 3;
    uno::Reference < sheet::XViewPane > xViewPane(init(),UNO_QUERY_THROW);
    xViewPane->setFirstVisibleColumn(col);
    xViewPane->setFirstVisibleRow(row);

    table::CellRangeAddress aCellRangeAddress = xViewPane->getVisibleRange();
    bool bResult = aCellRangeAddress.Sheet == 0;
    bResult &= aCellRangeAddress.StartRow == row;
    bResult &= aCellRangeAddress.StartColumn == col;
    CPPUNIT_ASSERT_MESSAGE("getVisibleRange()", bResult);

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
