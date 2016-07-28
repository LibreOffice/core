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

#include <cstddef>
#include <sal/types.h>

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include "../../source/app/cmdlineargs.hxx"

namespace {

class Test: public ::CppUnit::TestFixture {
public:
    void testTdf100837();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testTdf100837);
    CPPUNIT_TEST_SUITE_END();
};

class TestSupplier : public desktop::CommandLineArgs::Supplier {
public:
    virtual ~TestSupplier() {}
    virtual boost::optional< OUString > getCwdUrl() override { return boost::optional< OUString >(); }
    virtual bool next(OUString * argument) override {
        CPPUNIT_ASSERT(argument != nullptr);
        if (m_index < m_args.size()) {
            *argument = m_args[m_index++];
            return true;
        }
        else {
            return false;
        }
    }
    TestSupplier& operator << (const OUString& arg) { m_args.push_back(arg); return *this; }
private:
    std::vector< OUString > m_args;
    std::vector< OUString >::size_type m_index = 0;
};

// Test Office URI Schemes support
void Test::testTdf100837() {
    auto xContext = ::cppu::defaultBootstrap_InitialComponentContext();
    ::css::uno::Reference<::css::lang::XMultiComponentFactory> xFactory(xContext->getServiceManager());
    ::css::uno::Reference<::css::lang::XMultiServiceFactory> xSM(xFactory, ::css::uno::UNO_QUERY_THROW);
    // Without this we're crashing because callees are using getProcessServiceFactory
    ::comphelper::setProcessServiceFactory(xSM);

    {
        // 1. Test default behaviour: Office URIs define open mode
        TestSupplier supplier;
        supplier << "foo" << "ms-word:ofe|u|bar1" << "ms-word:ofv|u|bar2" << "ms-word:nft|u|bar3" << "baz";
        desktop::CommandLineArgs args(supplier);
        auto vOpenList      = args.GetOpenList();
        auto vForceOpenList = args.GetForceOpenList();
        auto vViewList      = args.GetViewList();
        auto vForceNewList  = args.GetForceNewList();
        // 2 documents go to Open list: foo; baz
        CPPUNIT_ASSERT_EQUAL(decltype(vOpenList.size())(2), vOpenList.size());
        CPPUNIT_ASSERT_EQUAL(OUString("foo"), vOpenList[0]);
        CPPUNIT_ASSERT_EQUAL(OUString("baz"), vOpenList[1]);
        // 1 document goes to ForceOpen list: bar1
        CPPUNIT_ASSERT_EQUAL(decltype(vForceOpenList.size())(1), vForceOpenList.size());
        CPPUNIT_ASSERT_EQUAL(OUString("bar1"), vForceOpenList[0]);
        // 1 document goes to View list: bar2
        CPPUNIT_ASSERT_EQUAL(decltype(vViewList.size())(1), vViewList.size());
        CPPUNIT_ASSERT_EQUAL(OUString("bar2"), vViewList[0]);
        // 1 document goes to ForceNew list: bar3
        CPPUNIT_ASSERT_EQUAL(decltype(vForceNewList.size())(1), vForceNewList.size());
        CPPUNIT_ASSERT_EQUAL(OUString("bar3"), vForceNewList[0]);
    }

    {
        // 2. Test --view. It overrides everything.
        TestSupplier supplier;
        supplier << "--view" << "foo" << "ms-word:ofe|u|bar1" << "ms-word:ofv|u|bar2" << "ms-word:nft|u|bar3" << "baz";
        desktop::CommandLineArgs args(supplier);
        auto vViewList = args.GetViewList();
        // All 5 documents go to View list
        CPPUNIT_ASSERT_EQUAL(decltype(vViewList.size())(5), vViewList.size());
    }

    {
        // 3. Test -o. It doesn't affect ofv and nft
        TestSupplier supplier;
        supplier << "-o" << "foo" << "ms-word:ofe|u|bar1" << "ms-word:ofv|u|bar2" << "ms-word:nft|u|bar3" << "baz";
        desktop::CommandLineArgs args(supplier);
        auto vViewList      = args.GetViewList();
        auto vForceOpenList = args.GetForceOpenList();
        auto vForceNewList  = args.GetForceNewList();
        // 1 document goes to View list: bar2
        CPPUNIT_ASSERT_EQUAL(decltype(vViewList.size())(1), vViewList.size());
        CPPUNIT_ASSERT_EQUAL(OUString("bar2"), vViewList[0]);
        // 3 documents go to ForceOpen list: foo, bar1, baz
        CPPUNIT_ASSERT_EQUAL(decltype(vForceOpenList.size())(3), vForceOpenList.size());
        CPPUNIT_ASSERT_EQUAL(OUString("foo"),  vForceOpenList[0]);
        CPPUNIT_ASSERT_EQUAL(OUString("bar1"), vForceOpenList[1]);
        CPPUNIT_ASSERT_EQUAL(OUString("baz"),  vForceOpenList[2]);
        // 1 document goes to ForceNew list: bar3
        CPPUNIT_ASSERT_EQUAL(decltype(vForceNewList.size())(1), vForceNewList.size());
        CPPUNIT_ASSERT_EQUAL(OUString("bar3"), vForceNewList[0]);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
