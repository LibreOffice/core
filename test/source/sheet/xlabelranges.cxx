/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xlabelranges.hxx>

#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XLabelRanges::testAddNew()
{
    uno::Reference<sheet::XLabelRanges> xLabelRanges(init(), UNO_QUERY_THROW);

    const sal_Int32 nCount = xLabelRanges->getCount();
    xLabelRanges->addNew(table::CellRangeAddress(0, 1, 0, 1, 0),
                         table::CellRangeAddress(0, 1, 1, 1, 6));

    CPPUNIT_ASSERT_MESSAGE("Unable to add new range", nCount < xLabelRanges->getCount());
}

void XLabelRanges::testRemoveByIndex()
{
    uno::Reference<sheet::XLabelRanges> xLabelRanges(init(), UNO_QUERY_THROW);

    const sal_Int32 nCount = xLabelRanges->getCount();
    xLabelRanges->removeByIndex(nCount - 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to remove range", (nCount - 1), xLabelRanges->getCount());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
