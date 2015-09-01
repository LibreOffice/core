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




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"
#include "gtest/gtest.h"
#include "stringhelper.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>

namespace rtl_OUStringBuffer
{


class insertUtf32 : public ::testing::Test
{
public:
    // initialise your test code values here.
    void SetUp()
    {
    }

    void TearDown()
    {
    }
}; // class getToken

// -----------------------------------------------------------------------------

TEST_F(insertUtf32, insertUtf32_001)
{
    ::rtl::OUStringBuffer aUStrBuf(4);
    aUStrBuf.insertUtf32(0,0x10ffff);

    rtl::OUString suStr = aUStrBuf.makeStringAndClear();
    rtl::OUString suStr2 = rtl::Uri::encode(suStr, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);

    rtl::OString sStr;
    sStr <<= suStr2;
    printf("%s\n", sStr.getStr());

    ASSERT_TRUE(sStr.equals(rtl::OString("%F4%8F%BF%BF")) == sal_True)
        << "Strings must be '%F4%8F%BF%BF'";
}

TEST_F(insertUtf32, insertUtf32_002)
{
    ::rtl::OUStringBuffer aUStrBuf(4);
    aUStrBuf.insertUtf32(0,0x41);
    aUStrBuf.insertUtf32(1,0x42);
    aUStrBuf.insertUtf32(2,0x43);

    rtl::OUString suStr = aUStrBuf.makeStringAndClear();
    rtl::OUString suStr2 = rtl::Uri::encode(suStr, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);

    rtl::OString sStr;
    sStr <<= suStr2;
    printf("%s\n", sStr.getStr());

    ASSERT_TRUE(sStr.equals(rtl::OString("ABC")) == sal_True)
        << "Strings must be 'ABC'";
}

} // namespace rtl_OUStringBuffer

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
