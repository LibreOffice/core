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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_testshl2.hxx"
#include "rtl/bootstrap.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppunit/simpleheader.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/xml/sax/XParser.hpp>

namespace css = com::sun::star;
namespace lang = css::lang;
namespace uno  = css::uno;
namespace sax = css::xml::sax;


// StringHelper
inline void operator <<= (rtl::OString& _rAsciiString, const rtl::OUString& _rUnicodeString)
{
    _rAsciiString = rtl::OUStringToOString(_rUnicodeString,RTL_TEXTENCODING_ASCII_US);
}


namespace unotest
{
//------------------------------------------------------------------------
// testing constructors
//------------------------------------------------------------------------

    class  ctor : public CppUnit::TestFixture
    {
    public:
        void ctor_001()
            {
                uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
                if (xFactory.is())
                {
                    // sample, get a xParser instance
                    uno::Reference< sax::XParser > xParser;
                    xParser = uno::Reference< sax::XParser > (
                        xFactory->createInstance(
                            ::rtl::OUString::createFromAscii("com.sun.star.xml.sax.Parser")), uno::UNO_QUERY);

                    CPPUNIT_ASSERT_MESSAGE("can't get sax::Parser", xParser.is());
                }
            }
        void ctor_002()
            {
                CPPUNIT_ASSERT_MESSAGE("Simple testshl2 test, MUST fail.", sal_False);
            }

        CPPUNIT_TEST_SUITE(ctor);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST(ctor_002);
        CPPUNIT_TEST_SUITE_END();
    };

    // -----------------------------------------------------------------------------
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(unotest::ctor, "unotest");
} // unotest

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.


void RegisterAdditionalFunctions(FktRegFuncPtr _pFunc)
{
    (void) _pFunc;
    uno::Reference<lang::XMultiServiceFactory> xMS;

    try
    {
        uno::Reference< uno::XComponentContext > xComponentContext = cppu::defaultBootstrap_InitialComponentContext();
        xMS.set(xComponentContext->getServiceManager(), uno::UNO_QUERY);
        comphelper::setProcessServiceFactory(xMS);
    }
    catch (::com::sun::star::uno::Exception e )
    {
        rtl::OString aError;
        aError <<= e.Message;
        printf("Error: %s\n", aError.getStr());
    }
}
// NOADDITIONAL;
