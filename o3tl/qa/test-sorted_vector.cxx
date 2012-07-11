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

#include <o3tl/sorted_vector.hxx>

using namespace ::o3tl;


// helper class
class SwContent
{
public:
    int x;

    SwContent(int x_) : x(x_) {}

    bool operator<( const SwContent &rCmp) const
    {
        return x < rCmp.x;
    }
};

class sorted_vector_test : public CppUnit::TestFixture
{
public:
    void testBasics()
    {
        o3tl::sorted_vector<SwContent*, o3tl::less_ptr_to<SwContent> > aVec;
        SwContent *p1 = new SwContent(1);
        SwContent *p2 = new SwContent(2);
        SwContent *p3 = new SwContent(3);
        SwContent *p4 = new SwContent(4);

        CPPUNIT_ASSERT( aVec.insert(p3).second );
        CPPUNIT_ASSERT( aVec.insert(p1).second );
        CPPUNIT_ASSERT( !aVec.insert(p3).second );

        CPPUNIT_ASSERT( aVec.size() == 2 );

        CPPUNIT_ASSERT( aVec[0] == p1 );
        CPPUNIT_ASSERT( aVec[1] == p3 );

        CPPUNIT_ASSERT( aVec.find(p1) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p1) - aVec.begin() == 0 );
        CPPUNIT_ASSERT( aVec.find(p3) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p3) - aVec.begin() == 1 );
        CPPUNIT_ASSERT( aVec.find(p2) == aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p4) == aVec.end() );

        CPPUNIT_ASSERT( aVec.erase(p1) == 1 );
        CPPUNIT_ASSERT( aVec.size() == 1 );
        CPPUNIT_ASSERT( aVec.erase(p2) == 0 );

        aVec.DeleteAndDestroyAll();
    }


    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(sorted_vector_test);
    CPPUNIT_TEST(testBasics);
    CPPUNIT_TEST_SUITE_END();
};

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(sorted_vector_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
