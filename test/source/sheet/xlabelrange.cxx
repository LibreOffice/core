/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xlabelrange.hxx>
#include <test/cppunitasserthelper.hxx>

#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XLabelRange::testGetSetDataArea()
{
    uno::Reference<sheet::XLabelRange> xLabelRange(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get DataArea", table::CellRangeAddress(0, 0, 0, 0, 1),
                                 xLabelRange->getDataArea());

    table::CellRangeAddress aDataAddr(1, 0, 1, 0, 8);
    xLabelRange->setDataArea(aDataAddr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set DataArea", aDataAddr, xLabelRange->getDataArea());
}

void XLabelRange::testGetSetLabelArea()
{
    uno::Reference<sheet::XLabelRange> xLabelRange(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get DataArea", table::CellRangeAddress(0, 0, 1, 0, 6),
                                 xLabelRange->getLabelArea());

    table::CellRangeAddress aLabelAddr(1, 0, 1, 0, 8);
    xLabelRange->setLabelArea(aLabelAddr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set LabelArea", aLabelAddr,
                                 xLabelRange->getLabelArea());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
