/*
 * LLA: 20040527
 */

#ifndef CPPUNIT_SIMPLEHEADER_HXX
#include <cppunit/simpleheader.hxx>
#endif

namespace skeleton
{
//------------------------------------------------------------------------
// testing constructors
//------------------------------------------------------------------------

    class  ctor : public CppUnit::TestFixture
    {
    public:
        ctor()
            {
                printf("ctor: called\n");
            }

        void ctor_test()
            {
                printf("ctor test: called\n");
            }
        void ctor_test2()
            {
                printf("ctor test2: called\n");
            }
        void ctor_test3()
            {
                printf("ctor test3: called\n");
            }

        CPPUNIT_TEST_SUITE(ctor);
        CPPUNIT_TEST(ctor_test);
        CPPUNIT_TEST(ctor_test2);
        CPPUNIT_TEST(ctor_test3);
        CPPUNIT_TEST_SUITE_END();
    };

    // -----------------------------------------------------------------------------
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(skeleton::ctor, "skeleton");
} // unotest

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.

NOADDITIONAL;
