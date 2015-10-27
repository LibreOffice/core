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



#include "precompiled_sal.hxx"
#include "sal/config.h"

#include "gtest/gtest.h"
#include "rtl/byteseq.hxx"
#include "sal/types.h"

namespace {

class Test: public ::testing::Test {
public:
};

TEST_F(Test, test_default) {
    rtl::ByteSequence s;
    ASSERT_EQ(sal_Int32(0), s.getLength());
}

TEST_F(Test, test_size0) {
    rtl::ByteSequence s(sal_Int32(0));
    ASSERT_EQ(sal_Int32(0), s.getLength());
}

TEST_F(Test, test_size5) {
    rtl::ByteSequence s(5);
    sal_Int8 const * p = s.getConstArray();
    ASSERT_EQ(sal_Int32(5), s.getLength());
    ASSERT_EQ(sal_Int8(0), p[0]);
    ASSERT_EQ(sal_Int8(0), p[1]);
    ASSERT_EQ(sal_Int8(0), p[2]);
    ASSERT_EQ(sal_Int8(0), p[3]);
    ASSERT_EQ(sal_Int8(0), p[4]);
}

TEST_F(Test, test_noinit0) {
    rtl::ByteSequence s(0, rtl::BYTESEQ_NODEFAULT);
    ASSERT_EQ(sal_Int32(0), s.getLength());
}

TEST_F(Test, test_noinit5) {
    rtl::ByteSequence s(5, rtl::BYTESEQ_NODEFAULT);
    ASSERT_EQ(sal_Int32(5), s.getLength());
}

TEST_F(Test, test_elem0) {
    rtl::ByteSequence s(0, 0);
    ASSERT_EQ(sal_Int32(0), s.getLength());
}

TEST_F(Test, test_elem5) {
    sal_Int8 const a[5] = { 0, 1, 2, 3, 4 };
    rtl::ByteSequence s(a, 5);
    sal_Int8 const * p = s.getConstArray();
    ASSERT_EQ(sal_Int32(5), s.getLength());
    ASSERT_EQ(sal_Int8(0), p[0]);
    ASSERT_EQ(sal_Int8(1), p[1]);
    ASSERT_EQ(sal_Int8(2), p[2]);
    ASSERT_EQ(sal_Int8(3), p[3]);
    ASSERT_EQ(sal_Int8(4), p[4]);
}

TEST_F(Test, test_copy) {
    rtl::ByteSequence s1(5);
    {
        rtl::ByteSequence s2(s1);
        ASSERT_EQ(sal_Int32(5), s2.getLength());
        ASSERT_EQ(s1.getConstArray(), s2.getConstArray());
        ASSERT_EQ(s1.getHandle(), s2.getHandle());
        ASSERT_EQ(sal_Int32(2), s1.getHandle()->nRefCount);
    }
    ASSERT_EQ(sal_Int32(1), s1.getHandle()->nRefCount);
}

TEST_F(Test, test_fromC) {
    sal_Sequence c = { 1, 1, { 0 } };
    {
        rtl::ByteSequence s(&c);
        ASSERT_EQ(sal_Int32(1), s.getLength());
        ASSERT_EQ(
            static_cast< void const * >(c.elements),
            static_cast< void const * >(s.getConstArray()));
        ASSERT_EQ(&c, s.getHandle());
        ASSERT_EQ(sal_Int32(2), c.nRefCount);
    }
    ASSERT_EQ(sal_Int32(1), c.nRefCount);
}

TEST_F(Test, test_noacquire) {
    sal_Sequence c = { 2, 1, { 0 } };
    {
        rtl::ByteSequence s(&c, rtl::BYTESEQ_NOACQUIRE);
        ASSERT_EQ(sal_Int32(1), s.getLength());
        ASSERT_EQ(
            static_cast< void const * >(c.elements),
            static_cast< void const * >(s.getConstArray()));
        ASSERT_EQ(&c, s.getHandle());
        ASSERT_EQ(sal_Int32(2), c.nRefCount);
    }
    ASSERT_EQ(sal_Int32(1), c.nRefCount);
}

TEST_F(Test, test_getArray) {
    sal_Int8 const a[5] = { 0, 1, 2, 3, 4 };
    rtl::ByteSequence s1(a, 5);
    rtl::ByteSequence s2(s1);
    sal_Int8 * p = s2.getArray();
    p[2] = 10;
    ASSERT_EQ(sal_Int32(1), s1.getHandle()->nRefCount);
    ASSERT_EQ(sal_Int32(1), s2.getHandle()->nRefCount);
    ASSERT_EQ(sal_Int8(2), s1.getConstArray()[2]);
    ASSERT_EQ(sal_Int8(10), s2.getConstArray()[2]);
}

TEST_F(Test, test_same0) {
    rtl::ByteSequence s1;
    rtl::ByteSequence s2;
    ASSERT_EQ(sal_True, s1 == s2);
    ASSERT_EQ(sal_True, s2 == s1);
    ASSERT_EQ(sal_False, s1 != s2);
    ASSERT_EQ(sal_False, s2 != s1);
}

TEST_F(Test, test_diffLen) {
    sal_Int8 const a[5] = { 0, 1, 2, 3, 4 };
    rtl::ByteSequence s1(a, 5);
    rtl::ByteSequence s2(a, 4);
    ASSERT_EQ(sal_False, s1 == s2);
    ASSERT_EQ(sal_False, s2 == s1);
    ASSERT_EQ(sal_True, s1 != s2);
    ASSERT_EQ(sal_True, s2 != s1);
}

TEST_F(Test, test_diffElem) {
    sal_Int8 const a1[5] = { 0, 1, 2, 3, 4 };
    rtl::ByteSequence s1(a1, 5);
    sal_Int8 const a2[5] = { 0, 1, 10, 3, 4 };
    rtl::ByteSequence s2(a2, 5);
    ASSERT_EQ(sal_False, s1 == s2);
    ASSERT_EQ(sal_False, s2 == s1);
    ASSERT_EQ(sal_True, s1 != s2);
    ASSERT_EQ(sal_True, s2 != s1);
}


}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

