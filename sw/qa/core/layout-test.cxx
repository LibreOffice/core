/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2010 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <sal/config.h>
#include <unotest/macros_test.hxx>
#include <test/bootstrapfixture.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>

using namespace ::com::sun::star::uno;
using namespace com::sun::star;

/* Implementation of Macros test */

class SwLayoutTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    SwLayoutTest();

    void createFileURL(const OUString& aFileBase, const OUString& aFileExtension, OUString& rFilePath);

    virtual void setUp();
    virtual void tearDown();

    void test();

    CPPUNIT_TEST_SUITE(SwLayoutTest);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(test);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xWriterComponent;
    OUString m_aBaseString;
};

void SwLayoutTest::createFileURL(const OUString& aFileBase, const OUString& aFileExtension, OUString& rFilePath)
{
    OUString aSep("/");
    OUStringBuffer aBuffer( getSrcRootURL() );
    aBuffer.append(m_aBaseString).append(aSep).append(aFileExtension);
    aBuffer.append(aSep).append(aFileBase).append(aFileExtension);
    rFilePath = aBuffer.makeStringAndClear();
}

void SwLayoutTest::test()
{
    OUString aFilePath;
    OUString aFileBase("test.");
    OUString aExtension("odt");
    createFileURL(aFileBase, aExtension, aFilePath);
    uno::Reference< lang::XComponent > xComponent = loadFromDesktop(aFilePath, "com.sun.star.text.TextDocument");
    CPPUNIT_ASSERT(xComponent.is());
}


SwLayoutTest::SwLayoutTest()
      : m_aBaseString("/sw/qa/core/data")
{
}

void SwLayoutTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that SwGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xWriterComponent =
        getMultiServiceFactory()->createInstance(OUString(
        "com.sun.star.comp.Writer.TextDocument"));
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xWriterComponent.is());
    mxDesktop = com::sun::star::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) );
}

void SwLayoutTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xWriterComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwLayoutTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
