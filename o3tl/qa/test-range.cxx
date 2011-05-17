/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include <o3tl/range.hxx>
#include <vector>
#include <deque>



using o3tl::range;
using o3tl::make_range;
using o3tl::range_of;
using std::size_t;


class range_test : public CppUnit::TestFixture
{
public:

    void int_test()
    {
        range<int>
            t1(12,88);
        range<int>
            t2(33,33);

        // ctor
        CPPUNIT_ASSERT_MESSAGE("int ctor1", t1.begin() == 12);
        CPPUNIT_ASSERT_MESSAGE("int ctor2", t1.end() == 88);
        CPPUNIT_ASSERT_MESSAGE("int ctor3", t2.begin() == 33);
        CPPUNIT_ASSERT_MESSAGE("int ctor4", t2.end() == 33);

        // make_range
        CPPUNIT_ASSERT_MESSAGE("int make_range1", make_range(0,8).begin() == 0);
        CPPUNIT_ASSERT_MESSAGE("int make_range2", make_range(0,8).end() == 8);

        // size
        CPPUNIT_ASSERT_MESSAGE("int size1", t1.size() == size_t(t1.end() - t1.begin()) );
        CPPUNIT_ASSERT_MESSAGE("int size2", t2.size() == size_t(0) );

        // contains
        range<int>      t3(0,10);
        range<int>      t4(7, 15);
        range<int>      t5(12, 12);
        range<int>      t6(13, 77);
        range<int>      t7(87, 87);
        range<int>      t8(87, 88);
        range<int>      t9(88, 88);
        range<int>      t10(33, 120);
        range<int>      t11(90, 100);
        range<int>      t12(200,200);

        CPPUNIT_ASSERT_MESSAGE("int contains1", t1.contains(t1));
        CPPUNIT_ASSERT_MESSAGE("int contains2", t1.contains(t2));
        CPPUNIT_ASSERT_MESSAGE("int contains3", ! t1.contains(t3));
        CPPUNIT_ASSERT_MESSAGE("int contains4", ! t1.contains(t4));
        CPPUNIT_ASSERT_MESSAGE("int contains5", t1.contains(t5));
        CPPUNIT_ASSERT_MESSAGE("int contains6", t1.contains(t6));
        CPPUNIT_ASSERT_MESSAGE("int contains7", t1.contains(t7));
        CPPUNIT_ASSERT_MESSAGE("int contains8", t1.contains(t8));
        CPPUNIT_ASSERT_MESSAGE("int contains9", ! t1.contains(t9));
        CPPUNIT_ASSERT_MESSAGE("int contains10", ! t1.contains(t10));
        CPPUNIT_ASSERT_MESSAGE("int contains11", ! t1.contains(t11));
        CPPUNIT_ASSERT_MESSAGE("int contains12", ! t1.contains(t12));

        CPPUNIT_ASSERT_MESSAGE("int contains n1", t1.contains(50));
        CPPUNIT_ASSERT_MESSAGE("int contains n2", t1.contains(12));
        CPPUNIT_ASSERT_MESSAGE("int contains n3", t1.contains(87));
        CPPUNIT_ASSERT_MESSAGE("int contains n4", ! t1.contains(3));
        CPPUNIT_ASSERT_MESSAGE("int contains n5", ! t1.contains(11));
        CPPUNIT_ASSERT_MESSAGE("int contains n6", ! t1.contains(88));
        CPPUNIT_ASSERT_MESSAGE("int contains n7", ! t1.contains(100));

        // overlaps
        range<int>      t13(88,99);

        CPPUNIT_ASSERT_MESSAGE("int overlaps1", t1.overlaps(t1));
        CPPUNIT_ASSERT_MESSAGE("int overlaps2", t1.overlaps(t2));
        CPPUNIT_ASSERT_MESSAGE("int overlaps3", ! t1.overlaps(t3));
        CPPUNIT_ASSERT_MESSAGE("int overlaps4", t1.overlaps(t4));
        CPPUNIT_ASSERT_MESSAGE("int overlaps5", t1.overlaps(t5));
        CPPUNIT_ASSERT_MESSAGE("int overlaps6", t1.overlaps(t6));
        CPPUNIT_ASSERT_MESSAGE("int overlaps7", t1.overlaps(t7));
        CPPUNIT_ASSERT_MESSAGE("int overlaps8", t1.overlaps(t8));
        CPPUNIT_ASSERT_MESSAGE("int overlaps9", ! t1.overlaps(t9));
        CPPUNIT_ASSERT_MESSAGE("int overlaps10", t1.overlaps(t10));
        CPPUNIT_ASSERT_MESSAGE("int overlaps11", ! t1.overlaps(t11));
        CPPUNIT_ASSERT_MESSAGE("int overlaps12", ! t1.overlaps(t12));
        CPPUNIT_ASSERT_MESSAGE("int overlaps13", ! t1.overlaps(t13));

        // distance_to
        CPPUNIT_ASSERT_MESSAGE("int distance_to1", t1.distance_to(t13) == 0);
        CPPUNIT_ASSERT_MESSAGE("int distance_to2", t1.distance_to(t9) == 0);
        CPPUNIT_ASSERT_MESSAGE("int distance_to3", t1.distance_to(t11) == 2);
        CPPUNIT_ASSERT_MESSAGE("int distance_to4", t1.distance_to(t8) == -1);
        CPPUNIT_ASSERT_MESSAGE("int distance_to5", t1.distance_to(t3) == -88);
    }

    void iterator_test()
    {
        typedef std::vector<char>::const_iterator   test_it;
        const std::vector<char>            hv(200,'x');


        test_it hit1 = hv.begin() + 12;
        test_it hit2 = hv.begin() + 88;

        range<test_it>
            t1(hit1, hit2);
        range<test_it>
            t2(hv.begin()+33, hv.begin()+33);

        // ctor
        CPPUNIT_ASSERT_MESSAGE("ivec ctor1", t1.begin() == hit1);
        CPPUNIT_ASSERT_MESSAGE("ivec ctor2", t1.end() == hit2);
        CPPUNIT_ASSERT_MESSAGE("ivec ctor3", t2.begin() == hv.begin()+33);
        CPPUNIT_ASSERT_MESSAGE("ivec ctor4", t2.end() == hv.begin()+33);

        // make_range
        CPPUNIT_ASSERT_MESSAGE("ivec make_range1", make_range(hv.begin(), hv.begin()+8).begin() == hv.begin());
        CPPUNIT_ASSERT_MESSAGE("ivec make_range2", make_range(hv.begin(), hv.begin()+8).end() == hv.begin()+8);

        // size
        CPPUNIT_ASSERT_MESSAGE("ivec size1", t1.size() == size_t(t1.end() - t1.begin()) );
        CPPUNIT_ASSERT_MESSAGE("ivec size2", t2.size() == size_t(0) );

        // contains
        range<test_it>      t3(hv.begin(), hv.begin() + 10);
        range<test_it>      t4(hv.begin() + 7, hv.begin() + 15);
        range<test_it>      t5(hit1, hit1);
        range<test_it>      t6(hv.begin() + 13, hv.begin() + 77);
        range<test_it>      t7(hv.begin() + 87, hv.begin() + 87);
        range<test_it>      t8(hv.begin() + 87, hit2);
        range<test_it>      t9(hit2, hit2);
        range<test_it>      t10(hv.begin() + 33, hv.begin() + 120);
        range<test_it>      t11(hv.begin() + 90, hv.begin() + 100);
        range<test_it>      t12(hv.begin() + 200,hv.begin() + 200);

        CPPUNIT_ASSERT_MESSAGE("ivec contains1", t1.contains(t1));
        CPPUNIT_ASSERT_MESSAGE("ivec contains2", t1.contains(t2));
        CPPUNIT_ASSERT_MESSAGE("ivec contains3", ! t1.contains(t3));
        CPPUNIT_ASSERT_MESSAGE("ivec contains4", ! t1.contains(t4));
        CPPUNIT_ASSERT_MESSAGE("ivec contains5", t1.contains(t5));
        CPPUNIT_ASSERT_MESSAGE("ivec contains6", t1.contains(t6));
        CPPUNIT_ASSERT_MESSAGE("ivec contains7", t1.contains(t7));
        CPPUNIT_ASSERT_MESSAGE("ivec contains8", t1.contains(t8));
        CPPUNIT_ASSERT_MESSAGE("ivec contains9", ! t1.contains(t9));
        CPPUNIT_ASSERT_MESSAGE("ivec contains10", ! t1.contains(t10));
        CPPUNIT_ASSERT_MESSAGE("ivec contains11", ! t1.contains(t11));
        CPPUNIT_ASSERT_MESSAGE("ivec contains12", ! t1.contains(t12));

        CPPUNIT_ASSERT_MESSAGE("ivec contains n1", t1.contains(hv.begin() + 50));
        CPPUNIT_ASSERT_MESSAGE("ivec contains n2", t1.contains(hit1));
        CPPUNIT_ASSERT_MESSAGE("ivec contains n3", t1.contains(hv.begin() + 87));
        CPPUNIT_ASSERT_MESSAGE("ivec contains n4", ! t1.contains(hv.begin() + 3));
        CPPUNIT_ASSERT_MESSAGE("ivec contains n5", ! t1.contains(hv.begin() + 11));
        CPPUNIT_ASSERT_MESSAGE("ivec contains n6", ! t1.contains(hit2));
        CPPUNIT_ASSERT_MESSAGE("ivec contains n7", ! t1.contains(hv.begin() + 100));

        // overlaps
        range<test_it>      t13(hit2, hv.begin() + 99);

        CPPUNIT_ASSERT_MESSAGE("ivec overlaps1", t1.overlaps(t1));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps2", t1.overlaps(t2));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps3", ! t1.overlaps(t3));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps4", t1.overlaps(t4));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps5", t1.overlaps(t5));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps6", t1.overlaps(t6));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps7", t1.overlaps(t7));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps8", t1.overlaps(t8));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps9", ! t1.overlaps(t9));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps10", t1.overlaps(t10));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps11", ! t1.overlaps(t11));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps12", ! t1.overlaps(t12));
        CPPUNIT_ASSERT_MESSAGE("ivec overlaps13", ! t1.overlaps(t13));

        // distance_to
        CPPUNIT_ASSERT_MESSAGE("ivec distance_to1", t1.distance_to(t13) == 0);
        CPPUNIT_ASSERT_MESSAGE("ivec distance_to2", t1.distance_to(t8) == -1);
        CPPUNIT_ASSERT_MESSAGE("ivec distance_to3", t1.distance_to(t9) == 0);
        CPPUNIT_ASSERT_MESSAGE("ivec distance_to4", t1.distance_to(t11) == 2);
        CPPUNIT_ASSERT_MESSAGE("ivec distance_to5", t1.distance_to(t3) == -88);

        const std::vector< int* >   h2(20, (int*)0);
        std::deque< double >        h3(30, 0.0);

        CPPUNIT_ASSERT_MESSAGE("range_of1", range_of(h2).begin() == h2.begin());
        CPPUNIT_ASSERT_MESSAGE("range_of2", range_of(h3).end() == h3.end());
    }

    // insert your test code here.
    void global()
    {
        int_test();
        iterator_test();
    }


    // These macros are needed by auto register mechanism.
    CPPUNIT_TEST_SUITE(range_test);
    CPPUNIT_TEST(global);
    CPPUNIT_TEST_SUITE_END();
}; // class range_test

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(range_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
