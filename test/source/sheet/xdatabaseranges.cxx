/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xdatabaseranges.hxx>

#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XDatabaseRanges::testAddRemoveDbRanges()
{
    uno::Reference<sheet::XDatabaseRanges> xDbRanges(init(), UNO_QUERY_THROW);

    xDbRanges->addNewByName(u"addNewRange"_ustr, table::CellRangeAddress(0, 1, 2, 3, 4));
    CPPUNIT_ASSERT_MESSAGE("Unable to add new db range", xDbRanges->hasByName(u"addNewRange"_ustr));

    CPPUNIT_ASSERT_THROW_MESSAGE(
        "No exception thrown, when adding range with existing name",
        xDbRanges->addNewByName(u"addNewRange"_ustr, table::CellRangeAddress(0, 1, 2, 3, 4)),
        css::uno::RuntimeException);

    xDbRanges->removeByName(u"addNewRange"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Unable to remove db range", !xDbRanges->hasByName(u"addNewRange"_ustr));

    CPPUNIT_ASSERT_THROW_MESSAGE("No exception, when removing none-existing range",
                                 xDbRanges->removeByName(u"addNewRange"_ustr),
                                 css::uno::RuntimeException);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
