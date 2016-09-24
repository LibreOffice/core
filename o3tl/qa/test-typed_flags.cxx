/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include <o3tl/typed_flags_set.hxx>

using namespace ::o3tl;


enum class ConfigurationChangedHint { NONE, ONE, TWO };

namespace o3tl
{
    template<> struct typed_flags< ConfigurationChangedHint> : is_typed_flags< ConfigurationChangedHint, 0xFF> {};
}

class typed_flags_test : public CppUnit::TestFixture
{
public:
    void testBasics()
    {
        ConfigurationChangedHint nHint = ConfigurationChangedHint::ONE;

        CPPUNIT_ASSERT( ConfigurationChangedHint::ONE & ConfigurationChangedHint::ONE );
        CPPUNIT_ASSERT( nHint & ConfigurationChangedHint::ONE );
        CPPUNIT_ASSERT( ConfigurationChangedHint::ONE & nHint );

        CPPUNIT_ASSERT( ConfigurationChangedHint::ONE | ConfigurationChangedHint::ONE );
        CPPUNIT_ASSERT( nHint | ConfigurationChangedHint::ONE );
        CPPUNIT_ASSERT( ConfigurationChangedHint::ONE | nHint );

        CPPUNIT_ASSERT( ~nHint );
        CPPUNIT_ASSERT( ~ConfigurationChangedHint::ONE );

        nHint |= ConfigurationChangedHint::ONE;
        CPPUNIT_ASSERT( bool(nHint |= ConfigurationChangedHint::ONE) );

        nHint &= ConfigurationChangedHint::ONE;
        CPPUNIT_ASSERT( bool(nHint &= ConfigurationChangedHint::ONE) );

        CPPUNIT_ASSERT(
            !((ConfigurationChangedHint::NONE | ConfigurationChangedHint::ONE)
              & (ConfigurationChangedHint::NONE
                 | ConfigurationChangedHint::TWO)));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(typed_flags_test);
    CPPUNIT_TEST(testBasics);
    CPPUNIT_TEST_SUITE_END();
};


CPPUNIT_TEST_SUITE_REGISTRATION(typed_flags_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
