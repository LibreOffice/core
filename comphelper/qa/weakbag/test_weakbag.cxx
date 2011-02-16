/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_comphelper.hxx"
#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "comphelper/weakbag.hxx"
#include "cppuhelper/weak.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

namespace {

namespace css = com::sun::star;

class Test: public CppUnit::TestFixture {
public:
    void test() {
        css::uno::Reference< css::uno::XInterface > ref1(new cppu::OWeakObject);
        css::uno::Reference< css::uno::XInterface > ref2(new cppu::OWeakObject);
        css::uno::Reference< css::uno::XInterface > ref3(new cppu::OWeakObject);
        comphelper::WeakBag< css::uno::XInterface > bag;
        bag.add(ref1);
        bag.add(ref1);
        bag.add(ref2);
        bag.add(ref2);
        ref1.clear();
        bag.add(ref3);
        ref3.clear();
        CPPUNIT_ASSERT_MESSAGE("remove first ref2", bag.remove() == ref2);
        CPPUNIT_ASSERT_MESSAGE("remove second ref2", bag.remove() == ref2);
        CPPUNIT_ASSERT_MESSAGE("remove first null", !bag.remove().is());
        CPPUNIT_ASSERT_MESSAGE("remove second null", !bag.remove().is());
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
