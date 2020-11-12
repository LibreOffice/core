/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcelladdressable.hxx>
#include <test/cppunitasserthelper.hxx>

#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XCellAddressable::testGetCellAddress()
{
    uno::Reference<sheet::XCellAddressable> xCellAddressable(init(), UNO_QUERY_THROW);
    table::CellAddress xCellAddress = xCellAddressable->getCellAddress();
    table::CellAddress defaultCellAddress(0, 2, 3);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("getCellAddress() didn't returned default cell address",
                                 defaultCellAddress, xCellAddress);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
