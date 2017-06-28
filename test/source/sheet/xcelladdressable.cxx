/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcelladdressable.hxx>

#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star;
using namespace com::sun::star::uno;

CPPUNIT_NS_BEGIN

template<> struct assertion_traits<table::CellAddress>
{
    static bool equal(const table::CellAddress& x, const table::CellAddress& y)
    {
        return x == y;
    }

    static std::string toString( const table::CellAddress& x )
    {
        OStringStream ost;
        ost << "Sheet: " << x.Sheet << " "
            << "Column: " << x.Column << " "
            << "Row: " << x.Row;
        return ost.str();
    }
};

CPPUNIT_NS_END

namespace apitest {

void XCellAddressable::testGetCellAddress()
{
    uno::Reference< sheet::XCellAddressable > xCellAddressable(init(), UNO_QUERY_THROW);
    table::CellAddress xCellAddress = xCellAddressable->getCellAddress();
    table::CellAddress defaultCellAddress;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("getCellAddress() didn't returned default cell address",
                                 defaultCellAddress, xCellAddress);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
