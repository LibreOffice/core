/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <random>

#include <test/sheet/xfunctiondescriptions.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/XFunctionDescriptions.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XFunctionDescriptions::testGetById()
{
    uno::Reference<sheet::XFunctionDescriptions> xFD(init(), UNO_QUERY_THROW);

    const sal_Int32 nCount = xFD->getCount();
    CPPUNIT_ASSERT_MESSAGE("No FunctionDescriptions available", 0 < nCount);

    // first grab a random function descriptions
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, nCount - 1);
    int nNumber = distr(gen);

    sal_Int32 aId1 = 0;
    OUString aName1;
    uno::Sequence<beans::PropertyValue> aProps1;
    CPPUNIT_ASSERT(xFD->getByIndex(nNumber) >>= aProps1);
    for (const auto& aProp : std::as_const(aProps1))
    {
        if (aProp.Name == "Id")
            aId1 = aProp.Value.get<sal_Int32>();
        if (aProp.Name == "Name")
            aName1 = aProp.Value.get<OUString>();
    }

    // fetch the same descriptions by its id
    sal_Int32 aId2 = 0;
    OUString aName2;
    const uno::Sequence<beans::PropertyValue> aProps2 = xFD->getById(aId1);
    CPPUNIT_ASSERT_MESSAGE("Received empty FunctionDescriptions from getById()",
                           aProps2.hasElements());
    for (const auto& aProp : aProps2)
    {
        if (aProp.Name == "Id")
            aId2 = aProp.Value.get<sal_Int32>();
        if (aProp.Name == "Name")
            aName2 = aProp.Value.get<OUString>();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Received wrong FunctionDescriptions (Id)", aId1, aId2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Received wrong FunctionDescriptions (Name)", aName1, aName2);

    CPPUNIT_ASSERT_THROW_MESSAGE("No IllegalArgumentException thrown", xFD->getById(-1),
                                 css::lang::IllegalArgumentException);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
