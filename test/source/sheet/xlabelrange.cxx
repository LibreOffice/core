/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xlabelrange.hxx>
#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include "cppunit/TestAssert.h"

using namespace css;
using namespace css::uno;

namespace apitest {

void XLabelRange::testDataArea()
{
    uno::Reference < sheet::XLabelRange > xLabelRange(init(),UNO_QUERY_THROW);
    table::CellRangeAddress setDAddress;
    table::CellRangeAddress gA = xLabelRange->getDataArea();
    xLabelRange->setDataArea(setDAddress);
    CPPUNIT_ASSERT( setDAddress==gA );

}

void XLabelRange::testLabelArea()
{
    uno::Reference < sheet::XLabelRange > xLabelRange(init(),UNO_QUERY_THROW);
    table::CellRangeAddress setLAddress;
    table::CellRangeAddress gA1 = xLabelRange->getLabelArea();
    xLabelRange->setLabelArea(setLAddress);
    CPPUNIT_ASSERT( setLAddress==gA1 );

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
