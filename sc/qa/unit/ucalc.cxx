/*
 * Known problems:
 * + We need to re-enable the exports.map with the right symbol
 */

// TODO ...
//    officecfg: can we move this into our skeleton ?
//          Solve the Setup.xcu problem pleasantly [ custom version ? ]
//    deliver.pl
//          don't call regcomp if we don't have it.
//              In an ideal world
//              a) scp2 goes away and logic moved into the deliver d.lst
//              b) install set gets built incrementally as the build progresses
//              c) the new .xml component registration stuff then removes
//                 the need for manually calling regcomp and knowing what
//                 services we need, and in what .so they are implemented

#include "sal/config.h"

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/svapp.hxx>
#include <scdll.hxx>
#include <document.hxx>

#include "preextstl.h"
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include "postextstl.h"

using namespace ::com::sun::star;

namespace {

class Test : public CppUnit::TestFixture {
public:
    // init
    virtual void setUp();
    virtual void tearDown();

    // tests
    void createDocument();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(createDocument);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< uno::XComponentContext > m_context;
};

void Test::setUp()
{
    m_context = cppu::defaultBootstrap_InitialComponentContext();

    uno::Reference<lang::XMultiComponentFactory> xFactory(m_context->getServiceManager());
    uno::Reference<lang::XMultiServiceFactory> xSM(xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(xSM);

    InitVCL(xSM);

    ScDLL::Init();
}

void Test::tearDown()
{
    uno::Reference< lang::XComponent >(m_context, uno::UNO_QUERY_THROW)->dispose();
}

void Test::createDocument()
{
    ScDocument *doc = new ScDocument();

    rtl::OUString aTabName = rtl::OUString::createFromAscii ("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            doc->InsertTab (0, aTabName));
    double val = 1;
    doc->SetValue (0, 0, 0, val);
    doc->SetValue (0, 1, 0, val);
    doc->SetString (0, 2, 0, rtl::OUString::createFromAscii ("=SUM(A1:A2)"));
    doc->CalcAll();
    double result;
    doc->GetValue (0, 2, 0, result);
    CPPUNIT_ASSERT_MESSAGE ("calculation failed", result == 2.0);

    delete doc;
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();
