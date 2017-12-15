/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcellrangeaddressable.hxx>

#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XCellRangeAddressable::testGetRangeAddress()
{
    uno::Reference<sheet::XCellRangeAddressable> xCRA(init(), UNO_QUERY_THROW);

    table::CellRangeAddress aCellRangeAddr = xCRA->getRangeAddress();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get CellRangeAddress", sal_Int16(0),
                                 aCellRangeAddr.Sheet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
