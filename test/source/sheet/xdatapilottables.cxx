/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xdatapilottables.hxx>

#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XDataPilotTables::testXDataPilotTables()
{
    uno::Reference<sheet::XDataPilotTables> xDPT(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(getXSpreadsheet(), UNO_QUERY_THROW);

    uno::Reference<sheet::XDataPilotDescriptor> xDPD(xDPT->createDataPilotDescriptor(),
                                                     UNO_SET_THROW);

    xDPT->insertNewByName(u"XDataPilotTables"_ustr, table::CellAddress(0, 9, 8), xDPD);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to insert new DataPilotTable", u"Filter"_ustr,
                                 xSheet->getCellByPosition(9, 8)->getFormula());

    CPPUNIT_ASSERT_THROW_MESSAGE(
        "No exception thrown, when inserting element with existing name",
        xDPT->insertNewByName(u"XDataPilotTables"_ustr, table::CellAddress(0, 7, 7), xDPD),
        css::uno::RuntimeException);

    xDPT->removeByName(u"XDataPilotTables"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to remove DataPilotTable", u""_ustr,
                                 xSheet->getCellByPosition(9, 8)->getFormula());

    CPPUNIT_ASSERT_THROW_MESSAGE("No exception, when removing no existing element",
                                 xDPT->removeByName(u"XDataPilotTables"_ustr),
                                 css::uno::RuntimeException);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
