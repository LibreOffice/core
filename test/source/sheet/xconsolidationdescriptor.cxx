/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xconsolidationdescriptor.hxx>
#include <test/cppunitasserthelper.hxx>

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/XConsolidationDescriptor.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XConsolidationDescriptor::testGetFunction()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get function", sheet::GeneralFunction_SUM,
                                 xConsolidationDescriptor->getFunction());
}

void XConsolidationDescriptor::testSetFunction()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    xConsolidationDescriptor->setFunction(sheet::GeneralFunction_COUNT);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set function", sheet::GeneralFunction_COUNT,
                                 xConsolidationDescriptor->getFunction());
}

void XConsolidationDescriptor::testGetSources()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    uno::Sequence<table::CellRangeAddress> aSources = xConsolidationDescriptor->getSources();
    CPPUNIT_ASSERT_MESSAGE("Unable to get sources", !aSources.hasElements());
}

void XConsolidationDescriptor::testSetSources()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    uno::Sequence<table::CellRangeAddress> aSources{ table::CellRangeAddress(0, 1, 1, 5, 5) };
    xConsolidationDescriptor->setSources(aSources);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set sources", table::CellRangeAddress(0, 1, 1, 5, 5),
                                 xConsolidationDescriptor->getSources()[0]);
}

void XConsolidationDescriptor::testGetStartOutputPosition()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get start output position", table::CellAddress(0, 0, 0),
                                 xConsolidationDescriptor->getStartOutputPosition());
}

void XConsolidationDescriptor::testSetStartOutputPosition()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    xConsolidationDescriptor->setStartOutputPosition(table::CellAddress(0, 1, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set start output position", table::CellAddress(0, 1, 2),
                                 xConsolidationDescriptor->getStartOutputPosition());
}

void XConsolidationDescriptor::testGetUseColumnHeaders()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Unable to get use column headers",
                           !xConsolidationDescriptor->getUseColumnHeaders());
}

void XConsolidationDescriptor::testSetUseColumnHeaders()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    xConsolidationDescriptor->setUseColumnHeaders(true);
    CPPUNIT_ASSERT_MESSAGE("Unable to set use column headers",
                           xConsolidationDescriptor->getUseColumnHeaders());
}

void XConsolidationDescriptor::testGetUseRowHeaders()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Unable to get use row headers",
                           !xConsolidationDescriptor->getUseRowHeaders());
}

void XConsolidationDescriptor::testSetUseRowHeaders()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    xConsolidationDescriptor->setUseRowHeaders(true);
    CPPUNIT_ASSERT_MESSAGE("Unable to set use row headers",
                           xConsolidationDescriptor->getUseRowHeaders());
}

void XConsolidationDescriptor::testGetInsertLinks()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Unable to get use insert links",
                           !xConsolidationDescriptor->getInsertLinks());
}

void XConsolidationDescriptor::testSetInsertLinks()
{
    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(init(),
                                                                             UNO_QUERY_THROW);

    xConsolidationDescriptor->setInsertLinks(true);
    CPPUNIT_ASSERT_MESSAGE("Unable to set use insert links",
                           xConsolidationDescriptor->getInsertLinks());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
