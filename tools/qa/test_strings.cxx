/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <tools/string.hxx>

namespace test {
    namespace unistring {
        /**
         * test::unistring::Compare Perform comparison functions
         * tests on UniString.
         */
        class Compare: public CppUnit::TestFixture
        {
        private:
            /**
             * Performs tests on natural comparison function
             */
            void testCompareToNumeric();

            CPPUNIT_TEST_SUITE(Compare);
            CPPUNIT_TEST(testCompareToNumeric);
            CPPUNIT_TEST_SUITE_END();

        };
    }
}

#define US_FROM_STRING(STRING) UniString((STRING), RTL_TEXTENCODING_UTF8)

void test::unistring::Compare::testCompareToNumeric()
{
// --- Some generic tests to ensure we do not alter original behavior
// outside what we want
    CPPUNIT_ASSERT(
        US_FROM_STRING("ABC").CompareToNumeric(US_FROM_STRING("ABC")) == COMPARE_EQUAL
    );
    // Case sensitivity
    CPPUNIT_ASSERT(
        US_FROM_STRING("ABC").CompareToNumeric(US_FROM_STRING("abc")) == COMPARE_LESS
    );
    // Reverse
    CPPUNIT_ASSERT(
        US_FROM_STRING("abc").CompareToNumeric(US_FROM_STRING("ABC")) == COMPARE_GREATER
    );
    // First shorter
    CPPUNIT_ASSERT(
        US_FROM_STRING("alongstring").CompareToNumeric(US_FROM_STRING("alongerstring")) == COMPARE_GREATER
    );
    // Second shorter
    CPPUNIT_ASSERT(
        US_FROM_STRING("alongerstring").CompareToNumeric(US_FROM_STRING("alongstring")) == COMPARE_LESS
    );
// -- Here we go on natural order, each one is followed by classic compare and the reverse comparison
    // That's why we originally made the patch
    CPPUNIT_ASSERT(
        US_FROM_STRING("Heading 9").CompareToNumeric(US_FROM_STRING("Heading 10")) == COMPARE_LESS
    );
    // Original behavior
    CPPUNIT_ASSERT(
        US_FROM_STRING("Heading 9").CompareTo(US_FROM_STRING("Heading 10")) == COMPARE_GREATER
    );
    CPPUNIT_ASSERT(
        US_FROM_STRING("Heading 10").CompareToNumeric(US_FROM_STRING("Heading 9")) == COMPARE_GREATER
    );
    // Harder
    CPPUNIT_ASSERT(
        US_FROM_STRING("July, the 4th").CompareToNumeric(US_FROM_STRING("July, the 10th")) == COMPARE_LESS
    );
    CPPUNIT_ASSERT(
        US_FROM_STRING("July, the 4th").CompareTo(US_FROM_STRING("July, the 10th")) == COMPARE_GREATER
    );
    CPPUNIT_ASSERT(
        US_FROM_STRING("July, the 10th").CompareToNumeric(US_FROM_STRING("July, the 4th")) == COMPARE_GREATER
    );
    // Hardest
    CPPUNIT_ASSERT(
        US_FROM_STRING("abc08").CompareToNumeric(US_FROM_STRING("abc010")) == COMPARE_LESS
    );
    CPPUNIT_ASSERT(
        US_FROM_STRING("abc08").CompareTo(US_FROM_STRING("abc010")) == COMPARE_GREATER
    );
    CPPUNIT_ASSERT(
        US_FROM_STRING("abc010").CompareToNumeric(US_FROM_STRING("abc08")) == COMPARE_GREATER
    );
}

CPPUNIT_TEST_SUITE_REGISTRATION(test::unistring::Compare);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
