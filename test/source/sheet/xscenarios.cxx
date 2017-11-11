/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xscenarios.hxx>

#include <com/sun/star/sheet/XScenarios.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XScenarios::testAddNewByName()
{
    uno::Reference<sheet::XScenarios> xScenarios(init(), UNO_QUERY_THROW);

    uno::Sequence<table::CellRangeAddress> aCellRangeAddresses(1);
    aCellRangeAddresses[0] = table::CellRangeAddress(0, 0, 0, 0, 0);

    xScenarios->addNewByName("XScenarios2", aCellRangeAddresses, "new");
    CPPUNIT_ASSERT_MESSAGE("Unable to add new XScenario", xScenarios->hasByName("XScenarios2"));
}

void XScenarios::testRemoveByName()
{
    uno::Reference<sheet::XScenarios> xScenarios(init(), UNO_QUERY_THROW);

    xScenarios->removeByName("XScenarios");
    CPPUNIT_ASSERT_MESSAGE("Unable to remove XScenario", !xScenarios->hasByName("XScenario"));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
