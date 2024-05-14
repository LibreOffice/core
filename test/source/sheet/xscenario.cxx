/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xscenario.hxx>

#include <com/sun/star/sheet/XScenario.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XScenario::testScenario()
{
    uno::Reference<sheet::XScenario> xScenario(getScenarioSpreadsheet(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Sheet is not a Scenario sheet", xScenario->getIsScenario());

    uno::Sequence<table::CellRangeAddress> aCellRangeAddr(1);
    xScenario->addRanges(aCellRangeAddr);
    xScenario->apply();

    CPPUNIT_ASSERT_MESSAGE("Unable add and apply ranges", xScenario->getIsScenario());
    CPPUNIT_ASSERT_MESSAGE("Unable execute getScenarioComment()",
                           !xScenario->getScenarioComment().isEmpty());

    xScenario->setScenarioComment(u"Test"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable execute setScenarioComment()", u"Test"_ustr,
                                 xScenario->getScenarioComment());
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
