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
