/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetcellrangecontainer.hxx>
#include <test/cppunitasserthelper.hxx>

#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XSheetCellRangeContainer::testAddRemoveRangeAddress()
{
    uno::Reference<sheet::XSheetCellRangeContainer> xSCRC(init(), UNO_QUERY_THROW);
    xSCRC->removeRangeAddresses(xSCRC->getRangeAddresses()); // prepare a clean slate
    uno::Sequence<table::CellRangeAddress> aAddr = createCellRangeAddresses();

    sal_Int32 cnt = xSCRC->getCount();
    xSCRC->addRangeAddress(aAddr[0], false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to add CellRangeAddress (count)", cnt + 1,
                                 xSCRC->getCount());

    uno::Sequence<table::CellRangeAddress> aAfterAddAddr = xSCRC->getRangeAddresses();
    cnt = xSCRC->getCount();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to add CellRangeAddress (entry)", aAddr[0],
                                 aAfterAddAddr[cnt - 1]);

    xSCRC->removeRangeAddress(aAddr[0]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to remove CellRangeAddress (count)", cnt - 1,
                                 xSCRC->getCount());

    const uno::Sequence<table::CellRangeAddress> aAfterRemoveAddr = xSCRC->getRangeAddresses();
    for (auto const& addr : aAfterRemoveAddr)
    {
        CPPUNIT_ASSERT_MESSAGE("Unable to remove CellRangeAddress (entry)", aAddr[0] != addr);
    }
}

void XSheetCellRangeContainer::testAddRemoveRangeAddresses()
{
    uno::Reference<sheet::XSheetCellRangeContainer> xSCRC(init(), UNO_QUERY_THROW);
    xSCRC->removeRangeAddresses(xSCRC->getRangeAddresses()); // prepare a clean slate
    uno::Sequence<table::CellRangeAddress> aAddr = createCellRangeAddresses();

    sal_Int32 cnt = xSCRC->getCount();
    xSCRC->addRangeAddresses(aAddr, false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to add CellRangeAddress (count)", cnt + 2,
                                 xSCRC->getCount());

    uno::Sequence<table::CellRangeAddress> aAfterAddAddr = xSCRC->getRangeAddresses();
    cnt = xSCRC->getCount();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to add CellRangeAddresses (entry: first)", aAddr[0],
                                 aAfterAddAddr[cnt - 2]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to add CellRangeAddresses (entry: second)", aAddr[1],
                                 aAfterAddAddr[cnt - 1]);

    xSCRC->removeRangeAddresses(aAddr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to remove CellRangeAddresses (count)", cnt - 2,
                                 xSCRC->getCount());

    const uno::Sequence<table::CellRangeAddress> aAfterRemoveAddr = xSCRC->getRangeAddresses();
    for (auto const& addr : aAfterRemoveAddr)
    {
        CPPUNIT_ASSERT_MESSAGE("Unable to remove CellRangeAddresses (entry: first)",
                               aAddr[0] != addr);
        CPPUNIT_ASSERT_MESSAGE("Unable to remove CellRangeAddresses (entry: second)",
                               aAddr[1] != addr);
    }
}

uno::Sequence<table::CellRangeAddress> XSheetCellRangeContainer::createCellRangeAddresses()
{
    uno::Sequence<table::CellRangeAddress> aAddr(2);
    auto pAddr = aAddr.getArray();
    for (unsigned int i = 0; i < 2; i++)
    {
        pAddr[i].Sheet = i;
        pAddr[i].StartColumn = i;
        pAddr[i].StartRow = i;
        pAddr[i].EndColumn = i + 3;
        pAddr[i].EndRow = i + 3;
    }

    return aAddr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
