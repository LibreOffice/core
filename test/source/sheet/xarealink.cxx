/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xarealink.hxx>

#include <com/sun/star/sheet/XAreaLink.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star;
using namespace com::sun::star::uno;

CPPUNIT_NS_BEGIN

template<> struct assertion_traits<table::CellRangeAddress>
{
    static bool equal(const table::CellRangeAddress& x, const table::CellRangeAddress& y)
    {
        return x == y;
    }

    static std::string toString( const table::CellRangeAddress& x )
    {
        OStringStream ost;
        ost << "Sheet: " << x.Sheet << " StartColumn: " << x.StartColumn << " StartRow: " << x.StartRow
            << " EndColumn: " << x.EndColumn << " EndRow: " << x.EndRow;
        return ost.str();
    }
};

CPPUNIT_NS_END

namespace apitest {

void XAreaLink::testSetDestArea()
{
    uno::Reference< sheet::XAreaLink > xAreaLink(init(), UNO_QUERY_THROW);

    xAreaLink->setDestArea(table::CellRangeAddress(1,3,4,5,8));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Couldn't set new dest area",
                                 table::CellRangeAddress(1,3,4,5,8), xAreaLink->getDestArea());
}

void XAreaLink::testSetSourceArea()
{
    uno::Reference< sheet::XAreaLink > xAreaLink(init(), UNO_QUERY_THROW);

    xAreaLink->setSourceArea("Sheet1.A1:B1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Couldn't set new source area",
                                 OUString("Sheet1.A1:B1"), xAreaLink->getSourceArea());
}

void XAreaLink::testGetDestArea()
{
    uno::Reference< sheet::XAreaLink > xAreaLink(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Couldn't get dest area",
                                 table::CellRangeAddress(1,2,3,2,3), xAreaLink->getDestArea());
}

void XAreaLink::testGetSourceArea()
{
    uno::Reference< sheet::XAreaLink > xAreaLink(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Couldn't get source area",
                                 OUString("a1:c1"), xAreaLink->getSourceArea());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
