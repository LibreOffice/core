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
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
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
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>

#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <com/sun/star/frame/XDesktop.hpp>

#include <string>
#include <iostream>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace {

bool compareFiles( const rtl::OUString& aFileNameOne, const rtl::OUString& aFileNameTwo)
{
    rtl::OString aOFileNameOne = rtl::OUStringToOString(aFileNameOne, RTL_TEXTENCODING_UTF8);
    std::ifstream aFileOne(aOFileNameOne.getStr());
    rtl::OString aOFileNameTwo = rtl::OUStringToOString(aFileNameTwo, RTL_TEXTENCODING_UTF8);
    std::ifstream aFileTwo(aOFileNameTwo.getStr());

    CPPUNIT_ASSERT_MESSAGE("files not open", aFileOne.is_open() && aFileTwo.is_open());

    sal_Int32 nLine = 1;
    while(!aFileOne.eof() && !aFileTwo.eof())
    {
        std::string aLineFileOne;
        std::string aLineFileTwo;

        std::getline(aFileOne, aLineFileOne);
        std::getline(aFileTwo, aLineFileTwo);

        if( aLineFileOne != aLineFileTwo)
        {
            rtl::OStringBuffer aErrorMessage("Mismatch between reference file and exported file in line ");
            aErrorMessage.append(nLine).append(".\nExpected: ");
            aErrorMessage.append(aLineFileOne.c_str()).append("\nFound   : ").append(aLineFileTwo.c_str());
            CPPUNIT_ASSERT_MESSAGE(aErrorMessage.getStr(), false);
        }
        nLine++;
    }
    return true;
}

}

class SwRegressionTest : public test::BootstrapFixture, unotest::MacrosTest
{
public:
    SwRegressionTest();

    virtual void setUp();
    virtual void tearDown();

    void test();

    CPPUNIT_TEST_SUITE(SwRegressionTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xWriterComponent;
    ::rtl::OUString m_aBaseString;
};

void SwRegressionTest::test()
{
    uno::Reference< lang::XComponent > xComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/data/odt/test.odt"));
    CPPUNIT_ASSERT(xComponent.is());

    uno::Reference< frame::XStorable > xStorable(xComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xStorable.is());

    uno::Sequence< beans::PropertyValue > aArgs(2);

    beans::PropertyValue aValue;
    uno::Any aAny;

    aAny <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("writer_layout_dump"));

    aValue.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
    aValue.Value = aAny;
    aValue.State = beans::PropertyState_DIRECT_VALUE;
    aArgs[0] = aValue;

    beans::PropertyValue aValue2;
    uno::Any aAny2;

    aAny2 <<= (sal_Bool)sal_True;

    aValue2.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Overwrite"));
    aValue2.Value = aAny2;
    aValue2.State = beans::PropertyState_DIRECT_VALUE;

    aArgs[1] = aValue2;

    xStorable->storeToURL(getURLFromSolver("/unittest/sw/test.xml"), aArgs);
}

SwRegressionTest::SwRegressionTest()
      : m_aBaseString(RTL_CONSTASCII_USTRINGPARAM("/sw/qa/core/data"))
{
}

void SwRegressionTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that SwGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xWriterComponent =
        getMultiServiceFactory()->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.TextDocument")));
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xWriterComponent.is());
    mxDesktop = Reference<com::sun::star::frame::XDesktop>( getMultiServiceFactory()->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))), UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE("", mxDesktop.is());
}

void SwRegressionTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xWriterComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwRegressionTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
