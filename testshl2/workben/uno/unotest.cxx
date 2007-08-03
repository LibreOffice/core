/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unotest.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 10:17:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_testshl2.hxx"
#ifndef _RTL_BOOTSTRAP_HXX_
#include "rtl/bootstrap.hxx"
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef CPPUNIT_SIMPLEHEADER_HXX
#include <cppunit/simpleheader.hxx>
#endif

#ifndef _CPPUHELPER_BOOTSTRAP_HXX_
#include <cppuhelper/bootstrap.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

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

        CPPUNIT_TEST_SUITE(ctor);
        CPPUNIT_TEST(ctor_001);
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
