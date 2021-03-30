/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_UNOTEST_BOOTSTRAPFIXTUREBASE_HXX
#define INCLUDED_UNOTEST_BOOTSTRAPFIXTUREBASE_HXX

#include <sal/config.h>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <unotest/detail/unotestdllapi.hxx>
#include <unotest/directories.hxx>

// For cppunit < 1.15.0.
#ifndef CPPUNIT_TEST_FIXTURE
#define CPPUNIT_TEST_FIXTURE(TestClass, TestName)                              \
    class TestName : public TestClass                                          \
    {                                                                          \
    public:                                                                    \
        void TestBody();                                                       \
        CPPUNIT_TEST_SUITE(TestName);                                          \
        CPPUNIT_TEST(TestBody);                                                \
        CPPUNIT_TEST_SUITE_END();                                              \
    };                                                                         \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName);                                 \
    void TestName::TestBody()
#endif

namespace test {

// Class to do lots of heavy-lifting UNO & environment
// bootstrapping for unit tests, such that we can use
// almost an entire LibreOffice during compile - so
// that we can get pieces of code alone to beat them up.

// NB. this class is instantiated multiple times during a
// run of unit tests ...
class OOO_DLLPUBLIC_UNOTEST BootstrapFixtureBase : public CppUnit::TestFixture
{
protected:
  Directories m_directories;
  css::uno::Reference<css::uno::XComponentContext> m_xContext;
  css::uno::Reference<css::lang::XMultiServiceFactory> m_xSFactory;
  css::uno::Reference<css::lang::XMultiComponentFactory> m_xFactory;

public:
  BootstrapFixtureBase();
  virtual ~BootstrapFixtureBase() override;

  const css::uno::Reference<css::uno::XComponentContext>&
              getComponentContext() const { return m_xContext; }
  const css::uno::Reference<css::lang::XMultiServiceFactory>&
              getMultiServiceFactory() const { return m_xSFactory; }

  virtual void setUp() override;
  virtual void tearDown() override;

  bool isWindowsRDP() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
