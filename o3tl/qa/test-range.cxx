/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "preextstl.h"
#include "gtest/gtest.h"
#include "postextstl.h"

#include <o3tl/range.hxx>
#include <vector>
#include <deque>



using o3tl::range;
using o3tl::make_range;
using o3tl::range_of;
using std::size_t;


class range_test : public ::testing::Test
{
public:

    void int_test()
    {
        range<int>
            t1(12,88);
        range<int>
            t2(33,33);

        // ctor
        ASSERT_TRUE(t1.begin() == 12) << "int ctor1";
        ASSERT_TRUE(t1.end() == 88) << "int ctor2";
        ASSERT_TRUE(t2.begin() == 33) << "int ctor3";
        ASSERT_TRUE(t2.end() == 33) << "int ctor4";

        // make_range
        ASSERT_TRUE(make_range(0,8).begin() == 0) << "int make_range1";
        ASSERT_TRUE(make_range(0,8).end() == 8) << "int make_range2";

        // size
        ASSERT_TRUE(t1.size() == size_t(t1.end() - t1.begin())) << "int size1";
        ASSERT_TRUE(t2.size() == size_t(0)) << "int size2";

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

        ASSERT_TRUE(t1.contains(t1)) << "int contains1";
        ASSERT_TRUE(t1.contains(t2)) << "int contains2";
        ASSERT_TRUE(! t1.contains(t3)) << "int contains3";
        ASSERT_TRUE(! t1.contains(t4)) << "int contains4";
        ASSERT_TRUE(t1.contains(t5)) << "int contains5";
        ASSERT_TRUE(t1.contains(t6)) << "int contains6";
        ASSERT_TRUE(t1.contains(t7)) << "int contains7";
        ASSERT_TRUE(t1.contains(t8)) << "int contains8";
        ASSERT_TRUE(! t1.contains(t9)) << "int contains9";
        ASSERT_TRUE(! t1.contains(t10)) << "int contains10";
        ASSERT_TRUE(! t1.contains(t11)) << "int contains11";
        ASSERT_TRUE(! t1.contains(t12)) << "int contains12";

        ASSERT_TRUE(t1.contains(50)) << "int contains n1";
        ASSERT_TRUE(t1.contains(12)) << "int contains n2";
        ASSERT_TRUE(t1.contains(87)) << "int contains n3";
        ASSERT_TRUE(! t1.contains(3)) << "int contains n4";
        ASSERT_TRUE(! t1.contains(11)) << "int contains n5";
        ASSERT_TRUE(! t1.contains(88)) << "int contains n6";
        ASSERT_TRUE(! t1.contains(100)) << "int contains n7";

        // overlaps
        range<int>      t13(88,99);

        ASSERT_TRUE(t1.overlaps(t1)) << "int overlaps1";
        ASSERT_TRUE(t1.overlaps(t2)) << "int overlaps2";
        ASSERT_TRUE(! t1.overlaps(t3)) << "int overlaps3";
        ASSERT_TRUE(t1.overlaps(t4)) << "int overlaps4";
        ASSERT_TRUE(t1.overlaps(t5)) << "int overlaps5";
        ASSERT_TRUE(t1.overlaps(t6)) << "int overlaps6";
        ASSERT_TRUE(t1.overlaps(t7)) << "int overlaps7";
        ASSERT_TRUE(t1.overlaps(t8)) << "int overlaps8";
        ASSERT_TRUE(! t1.overlaps(t9)) << "int overlaps9";
        ASSERT_TRUE(t1.overlaps(t10)) << "int overlaps10";
        ASSERT_TRUE(! t1.overlaps(t11)) << "int overlaps11";
        ASSERT_TRUE(! t1.overlaps(t12)) << "int overlaps12";
        ASSERT_TRUE(! t1.overlaps(t13)) << "int overlaps13";

        // distance_to
        ASSERT_TRUE(t1.distance_to(t13) == 0) << "int distance_to1";
        ASSERT_TRUE(t1.distance_to(t9) == 0) << "int distance_to2";
        ASSERT_TRUE(t1.distance_to(t11) == 2) << "int distance_to3";
        ASSERT_TRUE(t1.distance_to(t8) == -1) << "int distance_to4";
        ASSERT_TRUE(t1.distance_to(t3) == -88) << "int distance_to5";
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
        ASSERT_TRUE(t1.begin() == hit1) << "ivec ctor1";
        ASSERT_TRUE(t1.end() == hit2) << "ivec ctor2";
        ASSERT_TRUE(t2.begin() == hv.begin()+33) << "ivec ctor3";
        ASSERT_TRUE(t2.end() == hv.begin()+33) << "ivec ctor4";

        // make_range
        ASSERT_TRUE(make_range(hv.begin(), hv.begin()+8).begin() == hv.begin()) << "ivec make_range1";
        ASSERT_TRUE(make_range(hv.begin(), hv.begin()+8).end() == hv.begin()+8) << "ivec make_range2";

        // size
        ASSERT_TRUE(t1.size() == size_t(t1.end() - t1.begin())) << "ivec size1";
        ASSERT_TRUE(t2.size() == size_t(0)) << "ivec size2";

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

        ASSERT_TRUE(t1.contains(t1)) << "ivec contains1";
        ASSERT_TRUE(t1.contains(t2)) << "ivec contains2";
        ASSERT_TRUE(! t1.contains(t3)) << "ivec contains3";
        ASSERT_TRUE(! t1.contains(t4)) << "ivec contains4";
        ASSERT_TRUE(t1.contains(t5)) << "ivec contains5";
        ASSERT_TRUE(t1.contains(t6)) << "ivec contains6";
        ASSERT_TRUE(t1.contains(t7)) << "ivec contains7";
        ASSERT_TRUE(t1.contains(t8)) << "ivec contains8";
        ASSERT_TRUE(! t1.contains(t9)) << "ivec contains9";
        ASSERT_TRUE(! t1.contains(t10)) << "ivec contains10";
        ASSERT_TRUE(! t1.contains(t11)) << "ivec contains11";
        ASSERT_TRUE(! t1.contains(t12)) << "ivec contains12";

        ASSERT_TRUE(t1.contains(hv.begin() + 50)) << "ivec contains n1";
        ASSERT_TRUE(t1.contains(hit1)) << "ivec contains n2";
        ASSERT_TRUE(t1.contains(hv.begin() + 87)) << "ivec contains n3";
        ASSERT_TRUE(! t1.contains(hv.begin() + 3)) << "ivec contains n4";
        ASSERT_TRUE(! t1.contains(hv.begin() + 11)) << "ivec contains n5";
        ASSERT_TRUE(! t1.contains(hit2)) << "ivec contains n6";
        ASSERT_TRUE(! t1.contains(hv.begin() + 100)) << "ivec contains n7";

        // overlaps
        range<test_it>      t13(hit2, hv.begin() + 99);

        ASSERT_TRUE(t1.overlaps(t1)) << "ivec overlaps1";
        ASSERT_TRUE(t1.overlaps(t2)) << "ivec overlaps2";
        ASSERT_TRUE(! t1.overlaps(t3)) << "ivec overlaps3";
        ASSERT_TRUE(t1.overlaps(t4)) << "ivec overlaps4";
        ASSERT_TRUE(t1.overlaps(t5)) << "ivec overlaps5";
        ASSERT_TRUE(t1.overlaps(t6)) << "ivec overlaps6";
        ASSERT_TRUE(t1.overlaps(t7)) << "ivec overlaps7";
        ASSERT_TRUE(t1.overlaps(t8)) << "ivec overlaps8";
        ASSERT_TRUE(! t1.overlaps(t9)) << "ivec overlaps9";
        ASSERT_TRUE(t1.overlaps(t10)) << "ivec overlaps10";
        ASSERT_TRUE(! t1.overlaps(t11)) << "ivec overlaps11";
        ASSERT_TRUE(! t1.overlaps(t12)) << "ivec overlaps12";
        ASSERT_TRUE(! t1.overlaps(t13)) << "ivec overlaps13";

        // distance_to
        ASSERT_TRUE(t1.distance_to(t13) == 0) << "ivec distance_to1";
        ASSERT_TRUE(t1.distance_to(t8) == -1) << "ivec distance_to2";
        ASSERT_TRUE(t1.distance_to(t9) == 0) << "ivec distance_to3";
        ASSERT_TRUE(t1.distance_to(t11) == 2) << "ivec distance_to4";
        ASSERT_TRUE(t1.distance_to(t3) == -88) << "ivec distance_to5";

        const std::vector< int* >   h2(20, (int*)0);
        std::deque< double >        h3(30, 0.0);

        ASSERT_TRUE(range_of(h2).begin() == h2.begin()) << "range_of1";
        ASSERT_TRUE(range_of(h3).end() == h3.end()) << "range_of2";
    }

}; // class range_test

TEST_F(range_test, global)
{
    int_test();
    iterator_test();
}

