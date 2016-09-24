/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcellrangedata.hxx>
#include <com/sun/star/sheet/XCellRangeData.hpp>

#include "cppunit/extensions/HelperMacros.h"

using namespace css;
using namespace css::uno;

namespace apitest {

namespace {

void setValues(uno::Sequence< uno::Sequence < Any > >& rColRow, double nOffset)
{
    for (sal_Int32 i = 0; i < 4; ++i)
    {
        rColRow[i].realloc(4);
        for (sal_Int32 j = 0; j < 4; ++j)
        {
            Any& aAny = rColRow[i][j];
            double nValue = i + j + nOffset;
            aAny <<= nValue;
        }
    }
}

}

void XCellRangeData::testSetDataArray()
{
    uno::Reference< sheet::XCellRangeData > xCellRangeData( getXCellRangeData(), UNO_QUERY_THROW);

    uno::Sequence< uno::Sequence < Any > > aColRow;
    aColRow.realloc(4);
    setValues(aColRow, 1);
    xCellRangeData->setDataArray(aColRow);
    // need to check here for correct values

    // set old values
    setValues(aColRow, 0);
    xCellRangeData->setDataArray(aColRow);
}

void XCellRangeData::testGetDataArray()
{
    uno::Reference< sheet::XCellRangeData > xCellRangeData( getXCellRangeData(), UNO_QUERY_THROW);
    uno::Sequence< uno::Sequence < Any > > aColRow = xCellRangeData->getDataArray();
    for ( sal_Int32 i = 0; i < aColRow.getLength(); ++i)
    {
        for ( sal_Int32 j = 0; j < aColRow[i].getLength(); ++j)
        {
            Any& aAny = aColRow[i][j];
            double nValue = 0.0;
            CPPUNIT_ASSERT( aAny >>= nValue);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<double>(i+j), nValue, 0.000001);
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
