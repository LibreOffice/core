/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <comphelper/weakbag.hxx>
#include <cppuhelper/weak.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

namespace
{
// Tests the behaviour of comphelper::WeakBag
class WeakBagTest : public CppUnit::TestFixture
{
public:
    void test()
    {
        css::uno::Reference<css::uno::XInterface> ref1(new cppu::OWeakObject);
        css::uno::Reference<css::uno::XInterface> ref2(new cppu::OWeakObject);
        css::uno::Reference<css::uno::XInterface> ref3(new cppu::OWeakObject);

        comphelper::WeakBag<css::uno::XInterface> bag;

        bag.add(ref1);
        bag.add(ref1);
        bag.add(ref2);
        bag.add(ref2);
        ref1.clear();
        bag.add(ref3);
        ref3.clear();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("remove first ref2", bag.remove(), ref2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("remove second ref2", bag.remove(), ref2);
        CPPUNIT_ASSERT_MESSAGE("remove first null", !bag.remove().is());
        CPPUNIT_ASSERT_MESSAGE("remove second null", !bag.remove().is());
    }

    CPPUNIT_TEST_SUITE(WeakBagTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(WeakBagTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
