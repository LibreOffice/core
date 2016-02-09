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

    explicit SwContent(int x_) : x(x_) {}

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

        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), aVec.size() );

        CPPUNIT_ASSERT_EQUAL( p1, aVec[0] );
        CPPUNIT_ASSERT_EQUAL( p3, aVec[1] );

        CPPUNIT_ASSERT_EQUAL( p1, *aVec.begin() );
        CPPUNIT_ASSERT_EQUAL( p3, *(aVec.end()-1) );

        CPPUNIT_ASSERT_EQUAL( p1, aVec.front() );
        CPPUNIT_ASSERT_EQUAL( p3, aVec.back() );

        CPPUNIT_ASSERT( aVec.find(p1) != aVec.end() );
        CPPUNIT_ASSERT_EQUAL( static_cast<std::ptrdiff_t>(0), aVec.find(p1) - aVec.begin() );
        CPPUNIT_ASSERT( aVec.find(p3) != aVec.end() );
        CPPUNIT_ASSERT_EQUAL( static_cast<std::ptrdiff_t>(1), aVec.find(p3) - aVec.begin() );
        CPPUNIT_ASSERT( bool(aVec.find(p2) == aVec.end()) );
        CPPUNIT_ASSERT( bool(aVec.find(p4) == aVec.end()) );

        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), aVec.erase(p1) );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), aVec.size() );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), aVec.erase(p2) );

        aVec.DeleteAndDestroyAll();
        delete p1;
        delete p2;
        delete p4;
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

        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), aVec.erase(p1) );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), aVec.size() );

        aVec.erase(1);
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), aVec.size() );

        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), aVec.erase(p4) );

        aVec.clear();
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), aVec.size() );

        aVec.insert(p1);
        aVec.insert(p2);
        aVec.insert(p3);
        aVec.DeleteAndDestroyAll();
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), aVec.size() );
        delete p4;
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

        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(3), aVec2.size() );
        delete p1;
        delete p2;
        delete p3;
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

        CPPUNIT_ASSERT( bool(aVec.lower_bound(p1) == aVec.begin()) );
        CPPUNIT_ASSERT( bool(aVec.lower_bound(p4) == aVec.end()) );
        delete p1;
        delete p2;
        delete p3;
        delete p4;
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

        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), aVec.size() );

        CPPUNIT_ASSERT_EQUAL( p1, aVec[0] );
        CPPUNIT_ASSERT_EQUAL( p3, aVec[1] );

        CPPUNIT_ASSERT( aVec.insert(p2_2).second );
        CPPUNIT_ASSERT( aVec.insert(p2_3).second );
        CPPUNIT_ASSERT( !aVec.insert(p2_2).second );
        CPPUNIT_ASSERT( aVec.insert(p2_4).second );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(5), aVec.size() );

        CPPUNIT_ASSERT_EQUAL( p1, *aVec.begin() );
        CPPUNIT_ASSERT_EQUAL( p3, *(aVec.end()-1) );

        CPPUNIT_ASSERT_EQUAL( p1, aVec.front() );
        CPPUNIT_ASSERT_EQUAL( p3, aVec.back() );

        CPPUNIT_ASSERT( aVec.find(p1) != aVec.end() );
        CPPUNIT_ASSERT_EQUAL( static_cast<std::ptrdiff_t>(0), aVec.find(p1) - aVec.begin() );
        CPPUNIT_ASSERT( aVec.find(p3) != aVec.end() );
        CPPUNIT_ASSERT_EQUAL( static_cast<std::ptrdiff_t>(4), aVec.find(p3) - aVec.begin() );
        CPPUNIT_ASSERT( bool(aVec.find(p2) == aVec.end()) );
        CPPUNIT_ASSERT( bool(aVec.find(p4) == aVec.end()) );
        CPPUNIT_ASSERT( aVec.find(p2_2) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p2_2) - aVec.begin() >= 1 );
        CPPUNIT_ASSERT( aVec.find(p2_2) - aVec.begin() <  4 );
        CPPUNIT_ASSERT( aVec.find(p2_3) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p2_3) - aVec.begin() >= 1 );
        CPPUNIT_ASSERT( aVec.find(p2_3) - aVec.begin() <  4 );
        CPPUNIT_ASSERT( aVec.find(p2_4) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p2_4) - aVec.begin() >= 1 );
        CPPUNIT_ASSERT( aVec.find(p2_4) - aVec.begin() <  4 );

        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), aVec.erase(p1) );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(4), aVec.size() );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), aVec.erase(p2) );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), aVec.erase(p2_3) );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(3), aVec.size() );

        aVec.DeleteAndDestroyAll();
        delete p1;
        delete p2;
        delete p2_3;
        delete p4;
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

        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), aVec.erase(p1) );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(2), aVec.size() );

        aVec.erase(1);
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), aVec.size() );

        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), aVec.erase(p4) );

        aVec.clear();
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), aVec.size() );

        aVec.insert(p1);
        aVec.insert(p2);
        aVec.insert(p3);
        aVec.insert(p1_2);
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(4), aVec.size() );
        aVec.insert(p1_3);
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(5), aVec.size() );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), aVec.erase(p1) );
        CPPUNIT_ASSERT( bool(aVec.find(p1) == aVec.end()) );
        CPPUNIT_ASSERT( aVec.find(p1_2) != aVec.end() );
        CPPUNIT_ASSERT( aVec.find(p1_3) != aVec.end() );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), aVec.erase(p1_3) );
        CPPUNIT_ASSERT( bool(aVec.find(p1) == aVec.end()) );
        CPPUNIT_ASSERT( aVec.find(p1_2) != aVec.end() );
        CPPUNIT_ASSERT( bool(aVec.find(p1_3) == aVec.end()) );
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), aVec.erase(p1_3) );
        CPPUNIT_ASSERT( bool(aVec.find(p1) == aVec.end()) );
        CPPUNIT_ASSERT( aVec.find(p1_2) != aVec.end() );
        CPPUNIT_ASSERT( bool(aVec.find(p1_3) == aVec.end()) );

        aVec.DeleteAndDestroyAll();
        CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), aVec.size() );
        delete p1;
        delete p1_3;
        delete p4;
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


CPPUNIT_TEST_SUITE_REGISTRATION(sorted_vector_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
