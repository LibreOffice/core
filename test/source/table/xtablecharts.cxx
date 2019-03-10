/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/table/xtablecharts.hxx>

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XTableCharts.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void XTableCharts::testAddNewRemoveByName()
{
    uno::Reference<table::XTableCharts> xTC(init(), uno::UNO_QUERY_THROW);

    uno::Sequence<table::CellRangeAddress> aRanges(1);
    aRanges[0] = table::CellRangeAddress(0, 1, 1, 14, 4);
    xTC->addNewByName("XTableCharts", awt::Rectangle(500, 3000, 25000, 11000), aRanges, true, true);
    CPPUNIT_ASSERT(xTC->hasByName("XTableCharts"));

    xTC->removeByName("XTableCharts");
    CPPUNIT_ASSERT(!xTC->hasByName("XTableCharts"));
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
