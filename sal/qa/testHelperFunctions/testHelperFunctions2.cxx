#include <cppunit/simpleheader.hxx>
#include "stringhelper.hxx"

namespace testOfHelperFunctions
{
    class test_valueequal : public CppUnit::TestFixture
    {
    public:
        void valueequal_001();

        CPPUNIT_TEST_SUITE( test_valueequal );
        CPPUNIT_TEST( valueequal_001 );
        CPPUNIT_TEST_SUITE_END( );
    };

    void test_valueequal::valueequal_001( )
    {
        rtl::OString sValue;
        rtl::OUString suValue(rtl::OUString::createFromAscii("This is only a test of some helper functions"));
        sValue <<= suValue;
        t_print("'%s'\n", sValue.getStr());
    }

} // namespace testOfHelperFunctions

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( testOfHelperFunctions::test_valueequal, "helperFunctions" );

// -----------------------------------------------------------------------------
// This is only allowed to be in one file!
// NOADDITIONAL;
