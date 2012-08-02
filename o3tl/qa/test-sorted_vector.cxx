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

        CPPUNIT_ASSERT( *aVec.begin() == p1 );
        CPPUNIT_ASSERT( *(aVec.end()-1) == p3 );

        CPPUNIT_ASSERT( aVec.front() == p1 );
        CPPUNIT_ASSERT( aVec.back() == p3 );

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

    void testErase()
    {
        o3tl::sorted_vector<SwContent*, o3tl::less_ptr_to<SwContent> > aVec;
        SwContent *p1 = new SwContent(1);
        SwContent *p2 = new SwContent(2);
        SwContent *p3 = new SwContent(3);
        SwContent *p4 = new SwContent(4);

        aVec.insert(p1);
        aVec.insert(p2);
        aVec.insert(p3);

        CPPUNIT_ASSERT( aVec.erase(p1) == 1 );
        CPPUNIT_ASSERT( aVec.size() == 2 );

        aVec.erase(1);
        CPPUNIT_ASSERT( aVec.size() == 1 );

        CPPUNIT_ASSERT( aVec.erase(p4) == 0 );

        aVec.clear();
        CPPUNIT_ASSERT( aVec.size() == 0 );

        aVec.insert(p1);
        aVec.insert(p2);
        aVec.insert(p3);
        aVec.DeleteAndDestroyAll();
        CPPUNIT_ASSERT( aVec.size() == 0 );
    }

    void testInsertRange()
    {
        o3tl::sorted_vector<SwContent*, o3tl::less_ptr_to<SwContent> > aVec1;
        SwContent *p1 = new SwContent(1);
        SwContent *p2 = new SwContent(2);
        SwContent *p3 = new SwContent(3);

        aVec1.insert(p1);
        aVec1.insert(p2);
        aVec1.insert(p3);

        o3tl::sorted_vector<SwContent*, o3tl::less_ptr_to<SwContent> > aVec2;
        aVec2.insert( aVec1 );

        CPPUNIT_ASSERT( aVec2.size() == 3 );
    }

    void testLowerBound()
    {
        o3tl::sorted_vector<SwContent*, o3tl::less_ptr_to<SwContent> > aVec;
        SwContent *p1 = new SwContent(1);
        SwContent *p2 = new SwContent(2);
        SwContent *p3 = new SwContent(3);
        SwContent *p4 = new SwContent(4);

        aVec.insert(p1);
        aVec.insert(p2);
        aVec.insert(p3);

        CPPUNIT_ASSERT( aVec.lower_bound(p1) == aVec.begin() );
        CPPUNIT_ASSERT( aVec.lower_bound(p4) == aVec.end() );
    }

    void testBasics_FindPtr()
    {
        o3tl::sorted_vector<SwContent*, o3tl::less_ptr_to<SwContent>,
            o3tl::find_partialorder_ptrequals> aVec;
        SwContent *p1 = new SwContent(1);
        SwContent *p2 = new SwContent(2);
        SwContent *p2_2 = new SwContent(2);
        SwContent *p2_3 = new SwContent(2);
        SwContent *p2_4 = new SwContent(2);
        SwContent *p3 = new SwContent(3);
        SwContent *p4 = new SwContent(4);

        CPPUNIT_ASSERT( aVec.insert(p3).second );
        CPPUNIT_ASSERT( aVec.insert(p1).second );
        CPPUNIT_ASSERT( !aVec.insert(p3).second );

        CPPUNIT_ASSERT( aVec.size() == 2 );

        CPPUNIT_ASSERT( aVec[0] == p1 );
        CPPUNIT_ASSERT( aVec[1] == p3 );

        CPPUNIT_ASSERT( aVec.insert(p2_2).second );
        CPPUNIT_ASSERT( aVec.insert(p2_3).second );
        CPPUNIT_ASSERT( !aVec.insert(p2_2).second );
        CPPUNIT_ASSERT( aVec.insert(p2_4).second );
        CPPUNIT_ASSERT( aVec.size() == 5 );

        CPPUNIT_ASSERT( *aVec.begin() == p1 );
        CPPUNIT_ASSERT( *(aVec.end()-1) == p3 );

        CPPUNIT_ASSERT( aVec.front() == p1 );
        CPPUNIT_ASSERT( aVec.back() == p3 );

        CPPUNIT_ASSERT( aVec.find(p1) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p1) - aVec.begin() == 0 );
        CPPUNIT_ASSERT( aVec.find(p3) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p3) - aVec.begin() == 4 );
        CPPUNIT_ASSERT( aVec.find(p2) == aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p4) == aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p2_2) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p2_2) - aVec.begin() >= 1 );
        CPPUNIT_ASSERT( aVec.find(p2_2) - aVec.begin() <  4 );
        CPPUNIT_ASSERT( aVec.find(p2_3) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p2_3) - aVec.begin() >= 1 );
        CPPUNIT_ASSERT( aVec.find(p2_3) - aVec.begin() <  4 );
        CPPUNIT_ASSERT( aVec.find(p2_4) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p2_4) - aVec.begin() >= 1 );
        CPPUNIT_ASSERT( aVec.find(p2_4) - aVec.begin() <  4 );

        CPPUNIT_ASSERT( aVec.erase(p1) == 1 );
        CPPUNIT_ASSERT( aVec.size() == 4 );
        CPPUNIT_ASSERT( aVec.erase(p2) == 0 );
        CPPUNIT_ASSERT( aVec.erase(p2_3) == 1 );
        CPPUNIT_ASSERT( aVec.size() == 3 );

        aVec.DeleteAndDestroyAll();
    }

    void testErase_FindPtr()
    {
        o3tl::sorted_vector<SwContent*, o3tl::less_ptr_to<SwContent>,
            o3tl::find_partialorder_ptrequals> aVec;
        SwContent *p1 = new SwContent(1);
        SwContent *p1_2 = new SwContent(1);
        SwContent *p1_3 = new SwContent(1);
        SwContent *p2 = new SwContent(2);
        SwContent *p3 = new SwContent(3);
        SwContent *p4 = new SwContent(4);

        aVec.insert(p1);
        aVec.insert(p2);
        aVec.insert(p3);

        CPPUNIT_ASSERT( aVec.erase(p1) == 1 );
        CPPUNIT_ASSERT( aVec.size() == 2 );

        aVec.erase(1);
        CPPUNIT_ASSERT( aVec.size() == 1 );

        CPPUNIT_ASSERT( aVec.erase(p4) == 0 );

        aVec.clear();
        CPPUNIT_ASSERT( aVec.size() == 0 );

        aVec.insert(p1);
        aVec.insert(p2);
        aVec.insert(p3);
        aVec.insert(p1_2);
        CPPUNIT_ASSERT( aVec.size() == 4 );
        aVec.insert(p1_3);
        CPPUNIT_ASSERT( aVec.size() == 5 );
        CPPUNIT_ASSERT( aVec.erase(p1) == 1 );
        CPPUNIT_ASSERT( aVec.find(p1) == aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p1_2) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p1_3) != aVec.end() );
        CPPUNIT_ASSERT( aVec.erase(p1_3) == 1 );
        CPPUNIT_ASSERT( aVec.find(p1) == aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p1_2) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p1_3) == aVec.end() );
        CPPUNIT_ASSERT( aVec.erase(p1_3) == 0 );
        CPPUNIT_ASSERT( aVec.find(p1) == aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p1_2) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p1_3) == aVec.end() );

        aVec.DeleteAndDestroyAll();
        CPPUNIT_ASSERT( aVec.size() == 0 );
    }



    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(sorted_vector_test);
    CPPUNIT_TEST(testBasics);
    CPPUNIT_TEST(testErase);
    CPPUNIT_TEST(testInsertRange);
    CPPUNIT_TEST(testLowerBound);
    CPPUNIT_TEST(testBasics_FindPtr);
    CPPUNIT_TEST(testErase_FindPtr);
    CPPUNIT_TEST_SUITE_END();
};

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(sorted_vector_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
