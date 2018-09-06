/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <test/cppunitasserthelper.hxx>
#include <test/table/xtablechart.hxx>

#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XTableChart.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XTableChart::testSetGetHasColumnHeaders()
{
    uno::Reference<table::XTableChart> xTableChart(init(), UNO_QUERY_THROW);
    xTableChart->setHasColumnHeaders(true);
    CPPUNIT_ASSERT_MESSAGE("Successfully able to Set and Get Column Headers",
                           xTableChart->getHasColumnHeaders());
}

void XTableChart::testSetGetHasRowHeaders()
{
    uno::Reference<table::XTableChart> xTableChart(init(), UNO_QUERY_THROW);
    xTableChart->setHasRowHeaders(true);
    CPPUNIT_ASSERT_MESSAGE("Successfully able to Set and Get Row Headers",
                           xTableChart->getHasRowHeaders());
}

void XTableChart::testSetGetRanges()
{
    uno::Reference<table::XTableChart> xTableChart(init(), UNO_QUERY_THROW);
    uno::Sequence<table::CellRangeAddress> aRanges = xTableChart->getRanges();
    CPPUNIT_ASSERT_MESSAGE("Successfully able to Get Ranges", true);
    aRanges[0].EndRow = 7;
    xTableChart->setRanges(aRanges);
    uno::Sequence<table::CellRangeAddress> aRanges2 = xTableChart->getRanges();

    sal_Int32 nLength = aRanges->getlength();
    sal_Int32 nLength2 = aRanges2->getlength();
    if (nLength == nLength)
    {
        for (auto i = 0; i < nLength; i++)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to Set Values", aRanges[i],
                                         aRanges2[i]);
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
