/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <random>

#include <test/sheet/xrecentfunctions.hxx>

#include <com/sun/star/sheet/XRecentFunctions.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XRecentFunctions::testGetRecentFunctionIds()
{
    uno::Reference<sheet::XRecentFunctions> xRecentFunctions(init(), UNO_QUERY_THROW);

    uno::Sequence<sal_Int32> aIds = xRecentFunctions->getRecentFunctionIds();
    const sal_Int32 nNumber = aIds.getLength();
    CPPUNIT_ASSERT_MESSAGE("Recent IDs greater the max number",
                           nNumber <= xRecentFunctions->getMaxRecentFunctions());
    for (int i = 0; i < nNumber - 1; i++)
        for (int j = i + 1; j < nNumber; j++)
            CPPUNIT_ASSERT_MESSAGE("Same IDs found", aIds[i] != aIds[j]);
}

void XRecentFunctions::testSetRecentFunctionIds()
{
    uno::Reference<sheet::XRecentFunctions> xRecentFunctions(init(), UNO_QUERY_THROW);

    const sal_Int32 nMaxNumber = xRecentFunctions->getMaxRecentFunctions();

    // empty list
    uno::Sequence<sal_Int32> aIds;
    xRecentFunctions->setRecentFunctionIds(aIds);

    aIds = xRecentFunctions->getRecentFunctionIds();
    CPPUNIT_ASSERT_MESSAGE("Unable to set Ids (empty list)", !aIds.hasElements());

    // max. size list
    auto pIds = aIds.realloc(nMaxNumber);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, nMaxNumber + 1);

    int nStartIdx = distr(gen);
    for (int i = nStartIdx; i < nStartIdx + nMaxNumber; i++)
        pIds[i - nStartIdx] = 1;

    xRecentFunctions->setRecentFunctionIds(aIds);

    aIds = xRecentFunctions->getRecentFunctionIds();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set Ids (max. size list)", nMaxNumber,
                                 aIds.getLength());
}

void XRecentFunctions::testGetMaxRecentFunctions()
{
    uno::Reference<sheet::XRecentFunctions> xRecentFunctions(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Unable to execute getMaxRecentFunctions()",
                           sal_Int32(0) != xRecentFunctions->getMaxRecentFunctions());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
