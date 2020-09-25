/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/cppunitasserthelper.hxx>
#include <test/sheet/xcellrangereferrer.hxx>

#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest {

void XCellRangeReferrer::testGetReferredCells()
{
    uno::Reference< sheet::XCellRangeReferrer > xReferrer(init(), UNO_QUERY_THROW);
    uno::Reference< table::XCellRange > xReferredRange = xReferrer->getReferredCells();

    uno::Reference< sheet::XCellRangeAddressable > xAddressable( xReferredRange, UNO_QUERY_THROW );
    table::CellRangeAddress aCellRange = xAddressable->getRangeAddress();

    CPPUNIT_ASSERT_EQUAL(aCellRange, getCellRange());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
