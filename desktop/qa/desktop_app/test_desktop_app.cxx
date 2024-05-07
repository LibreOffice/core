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

#include <sal/types.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
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
    explicit TestSupplier(const std::initializer_list<OUString>& args) : m_args(args) {}

    virtual std::optional< OUString > getCwdUrl() override { return std::optional< OUString >(); }
    virtual bool next(OUString& argument) override {
        if (m_index < m_args.size()) {
            argument = m_args[m_index++];
            return true;
        }
        else {
            return false;
        }
    }
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
        TestSupplier supplier{ u"foo"_ustr, u"ms-word:ofe|u|bar1"_ustr, u"ms-word:ofv|u|bar2"_ustr, u"ms-word:nft|u|bar3"_ustr, u"baz"_ustr };
        desktop::CommandLineArgs args(supplier);
        auto vOpenList      = args.GetOpenList();
        auto vForceOpenList = args.GetForceOpenList();
        auto vViewList      = args.GetViewList();
        auto vForceNewList  = args.GetForceNewList();
        // 2 documents go to Open list: foo; baz
        CPPUNIT_ASSERT_EQUAL(decltype(vOpenList.size())(2), vOpenList.size());
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, vOpenList[0]);
        CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, vOpenList[1]);
        // 1 document goes to ForceOpen list: bar1
        CPPUNIT_ASSERT_EQUAL(decltype(vForceOpenList.size())(1), vForceOpenList.size());
        CPPUNIT_ASSERT_EQUAL(u"bar1"_ustr, vForceOpenList[0]);
        // 1 document goes to View list: bar2
        CPPUNIT_ASSERT_EQUAL(decltype(vViewList.size())(1), vViewList.size());
        CPPUNIT_ASSERT_EQUAL(u"bar2"_ustr, vViewList[0]);
        // 1 document goes to ForceNew list: bar3
        CPPUNIT_ASSERT_EQUAL(decltype(vForceNewList.size())(1), vForceNewList.size());
        CPPUNIT_ASSERT_EQUAL(u"bar3"_ustr, vForceNewList[0]);
    }

    {
        // 2. Test explicit open mode arguments. Office URI commands should have no effect
        TestSupplier supplier{ u"--view"_ustr, u"ms-word:ofe|u|foo"_ustr, u"-o"_ustr, u"ms-word:ofv|u|bar"_ustr, u"ms-word:nft|u|baz"_ustr };
        desktop::CommandLineArgs args(supplier);
        auto vViewList      = args.GetViewList();
        auto vForceOpenList = args.GetForceOpenList();
        // 1 document goes to View list: foo
        CPPUNIT_ASSERT_EQUAL(decltype(vViewList.size())(1), vViewList.size());
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, vViewList[0]);
        // 2 documents go to ForceOpen list: bar, baz
        CPPUNIT_ASSERT_EQUAL(decltype(vForceOpenList.size())(2), vForceOpenList.size());
        CPPUNIT_ASSERT_EQUAL(u"bar"_ustr,  vForceOpenList[0]);
        CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, vForceOpenList[1]);
    }

    {
        // 3. Test encoded URLs
        TestSupplier supplier{ u"foo"_ustr, u"ms-word:ofe%7Cu%7cbar1"_ustr, u"ms-word:ofv%7cu%7Cbar2"_ustr, u"ms-word:nft%7Cu%7cbar3"_ustr, u"baz"_ustr };
        desktop::CommandLineArgs args(supplier);
        auto vOpenList = args.GetOpenList();
        auto vForceOpenList = args.GetForceOpenList();
        auto vViewList = args.GetViewList();
        auto vForceNewList = args.GetForceNewList();
        // 2 documents go to Open list: foo; baz
        CPPUNIT_ASSERT_EQUAL(decltype(vOpenList.size())(2), vOpenList.size());
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, vOpenList[0]);
        CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, vOpenList[1]);
        // 1 document goes to ForceOpen list: bar1
        CPPUNIT_ASSERT_EQUAL(decltype(vForceOpenList.size())(1), vForceOpenList.size());
        CPPUNIT_ASSERT_EQUAL(u"bar1"_ustr, vForceOpenList[0]);
        // 1 document goes to View list: bar2
        CPPUNIT_ASSERT_EQUAL(decltype(vViewList.size())(1), vViewList.size());
        CPPUNIT_ASSERT_EQUAL(u"bar2"_ustr, vViewList[0]);
        // 1 document goes to ForceNew list: bar3
        CPPUNIT_ASSERT_EQUAL(decltype(vForceNewList.size())(1), vForceNewList.size());
        CPPUNIT_ASSERT_EQUAL(u"bar3"_ustr, vForceNewList[0]);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
