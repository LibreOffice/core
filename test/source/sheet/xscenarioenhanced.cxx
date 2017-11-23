/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xscenarioenhanced.hxx>
#include <test/cppunitasserthelper.hxx>

#include <com/sun/star/sheet/XScenarios.hpp>
#include <com/sun/star/sheet/XScenarioEnhanced.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XScenarioEnhanced::testGetRanges()
{
    uno::Reference<sheet::XScenarioEnhanced> xSE(getScenarioSpreadsheet(), UNO_QUERY_THROW);

    uno::Sequence<table::CellRangeAddress> aCellRangeAddr = xSE->getRanges();
    CPPUNIT_ASSERT_MESSAGE("No ranges found", aCellRangeAddr.getLength() != 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong CellRangeAddres", table::CellRangeAddress(1, 0, 0, 10, 10),
                                 aCellRangeAddr[0]);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
