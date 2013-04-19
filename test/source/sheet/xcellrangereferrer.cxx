/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcellrangereferrer.hxx>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

namespace apitest {

void XCellRangeReferrer::testGetReferredCells()
{
    uno::Reference< sheet::XCellRangeReferrer > xReferrer(init(), UNO_QUERY_THROW);
    uno::Reference< table::XCellRange > xReferredRange = xReferrer->getReferredCells();

    uno::Reference< sheet::XCellRangeAddressable > xAdressable( xReferredRange, UNO_QUERY_THROW );
    table::CellRangeAddress aCellRange = xAdressable->getRangeAddress();

    CPPUNIT_ASSERT_EQUAL( aCellRange.Sheet, maCellRange.Sheet );
    CPPUNIT_ASSERT_EQUAL( aCellRange.StartRow, maCellRange.StartRow );
    CPPUNIT_ASSERT_EQUAL( aCellRange.EndRow, maCellRange.EndRow );
    CPPUNIT_ASSERT_EQUAL( aCellRange.StartColumn, maCellRange.StartColumn );
    CPPUNIT_ASSERT_EQUAL( aCellRange.EndColumn, maCellRange.EndColumn );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
