/*
 * Known problems:
 *
 * + We need to work out why cppuint can't run this
 *    + can it run even the most trivial of tests ?
 *    + what symbol is it prodding around for ? and do we export it ?
 *      + cf. objdump -T on the generated .so
 *      + build verbose=1 to find it ...
 * + We are chancing our arm here; this is unlikely to work without
 * UNO bootstrapping - which is quite 'exciting' ;-)
 * + We need to resurrect the ubootstrap.[ch]xx files from old versions
 * of patches/test/build-in-unit-testing-sc.diff
 * + We need to re-enable the exports.map with the right symbol
 */


// TODO ...
//    officecfg: can we move this into our skeleton ?
//          Solve the Setup.xcu problem pleasantly [ custom version ? ]
//    Remove: Foo killed exception ! ...
//    deliver.pl
//          don't call regcomp if we don't have it.
//    find & kill signalfile stuff (?)
//    Consider - where to dump this code ?
//             - surely there is some Uno module we can use... ?
//             - 'unohelper' ?

#include "sal/config.h"
// #include "ubootstrap.hxx"
#include "vcl/svapp.hxx"
#include <scdll.hxx>
#include <document.hxx>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#define CATCH_FAIL(msg) \
    catch (const css::uno::Exception &e) { \
        t_print ("msg '%s'\n", rtl::OUStringToOString (e.Message, RTL_TEXTENCODING_UTF8).getStr()); \
        CPPUNIT_FAIL( msg ); \
        throw; \
    }

class Test: public CppUnit::TestFixture {
  //    UnitBootstrap *mpUnitBootstrap;
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
};

void Test::setUp()
{
// FIXME: we badly need to bootstrap UNO [!]
//    mpUnitBootstrap = new UnitBootstrap();
//    InitVCL (mpUnitBootstrap->getMSF());

    ScDLL::Init();
}

void Test::tearDown()
{
    DeInitVCL ();

    //    delete mpUnitBootstrap;
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
    fprintf (stderr, "one plus one = %g\n", result);
    CPPUNIT_ASSERT_MESSAGE ("calculation failed", result == 2.0);

    delete doc;
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");
