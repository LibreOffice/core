/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/table/xtablechart.hxx>
#include <test/cppunitasserthelper.hxx>

#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XTableChart.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void XTableChart::testGetSetHasColumnHeaders()
{
    uno::Reference<table::XTableChart> xTC(init(), uno::UNO_QUERY_THROW);

    xTC->setHasColumnHeaders(false);
    CPPUNIT_ASSERT(!xTC->getHasColumnHeaders());

    xTC->setHasColumnHeaders(true);
    CPPUNIT_ASSERT(xTC->getHasColumnHeaders());
}

void XTableChart::testGetSetHasRowHeaders()
{
    uno::Reference<table::XTableChart> xTC(init(), uno::UNO_QUERY_THROW);

    xTC->setHasRowHeaders(false);
    CPPUNIT_ASSERT(!xTC->getHasRowHeaders());

    xTC->setHasRowHeaders(true);
    CPPUNIT_ASSERT(xTC->getHasRowHeaders());
}

void XTableChart::testGetSetRanges()
{
    uno::Reference<table::XTableChart> xTC(init(), uno::UNO_QUERY_THROW);

    uno::Sequence<table::CellRangeAddress> aCRA = xTC->getRanges();
    aCRA[0].EndRow = 1;

    xTC->setRanges(aCRA);

    CPPUNIT_ASSERT_EQUAL(aCRA[0], xTC->getRanges()[0]);
}

} // namespace apitest
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
