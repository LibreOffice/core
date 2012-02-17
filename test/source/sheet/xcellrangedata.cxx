/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/sheet/xcellrangedata.hxx>
#include <com/sun/star/sheet/XCellRangeData.hpp>

#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

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
