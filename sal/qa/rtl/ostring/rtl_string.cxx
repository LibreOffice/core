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
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

namespace rtl_string
{

    class getLength : public ::testing::Test
    {
    public:
    }; // class getLength

    TEST_F(getLength, getLength_000)
    {
        rtl_string_getLength( NULL );
        // should not GPF
    }

    TEST_F(getLength, getLength_001)
    {
        rtl::OString aStr("Test Length.");
        sal_Int32 nValue = rtl_string_getLength( aStr.pData );

        ASSERT_TRUE(aStr.getLength() == nValue) << "Length must equal getLength()";
        ASSERT_TRUE(nValue >= 0
            && (strlen(aStr.getStr())
                == sal::static_int_cast< sal_uInt32 >(nValue))) << "Length must equal strlen()";
    }
// -----------------------------------------------------------------------------

    class newFromString : public ::testing::Test
    {
    public:
    }; // class newFromString

    // TEST_F(newFromString, newFromString_000)
    // {
    //     sal_Int32 nValue = rtl_string_newFromString( NULL, NULL );
    //     // should not GPF
    // }

    TEST_F(newFromString, newFromString_001)
    {
        rtl::OString aStr("Test Length.");
        rtl_String *pStr = NULL;

        rtl_string_newFromString( &pStr, aStr.pData );

        rtl::OString aNewStr(pStr);
        ASSERT_TRUE(aStr.equals(aNewStr) == sal_True) << "Strings must be equal";

        rtl_string_release(pStr);
    }

    // -----------------------------------------------------------------------------

    class convertUStringToString : public ::testing::Test
    {
    public:
    }; // class convertUStringToString


    // TEST_F(convertUStringToString, newFromString_000)
    // {
    //     sal_Int32 nValue = rtl_string_newFromString( NULL, NULL );
    //     // should not GPF
    // }

    TEST_F(convertUStringToString, convertUStringToString_001)
    {
        rtl::OUString suString = rtl::OUString::createFromAscii("Hello");
        rtl::OString sString;
        sal_Bool bRet = rtl_convertUStringToString(&sString.pData, suString.getStr(), suString.getLength(), RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS);

        ASSERT_TRUE(bRet == sal_True && sString.equals(rtl::OString("Hello")) == sal_True) << "Strings must be equal";
    }

    TEST_F(convertUStringToString, convertUStringToString_002)
    {
        rtl::OString sStr("H\xE4llo");
        rtl::OUString suString = rtl::OStringToOUString(sStr, RTL_TEXTENCODING_ISO_8859_15);

        rtl::OString sString;
        sal_Bool bRet = rtl_convertUStringToString(&sString.pData, suString.getStr(), suString.getLength(), RTL_TEXTENCODING_ISO_8859_15, OUSTRING_TO_OSTRING_CVTFLAGS);

        ASSERT_TRUE(bRet == sal_True && sString.equals(rtl::OString("H\xE4llo")) == sal_True) << "Strings must be equal";
    }

    TEST_F(convertUStringToString, convertUStringToString_003)
    {
        rtl::OString sStr("H\xC3\xA4llo");
        rtl::OUString suString = rtl::OStringToOUString(sStr, RTL_TEXTENCODING_UTF8);

        rtl::OString sString;
        sal_Bool bRet = rtl_convertUStringToString(&sString.pData, suString.getStr(), suString.getLength(), RTL_TEXTENCODING_ISO_8859_15, OUSTRING_TO_OSTRING_CVTFLAGS);

        ASSERT_TRUE(bRet == sal_True && sString.equals(rtl::OString("H\xE4llo")) == sal_True) << "Strings must be equal";
    }

    TEST_F(convertUStringToString, convertUStringToString_004)
    {
        rtl::OString sStr("Tsch\xFC\xDF");
        rtl::OUString suString = rtl::OStringToOUString(sStr, RTL_TEXTENCODING_ISO_8859_15);
        rtl::OString sString;

        sal_Bool       bRet = rtl_convertUStringToString(&sString.pData, suString.getStr(), suString.getLength(), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        /* sal_Bool */ bRet = rtl_convertUStringToString(&sString.pData, suString.getStr(), suString.getLength(), RTL_TEXTENCODING_ISO_8859_15, OUSTRING_TO_OSTRING_CVTFLAGS);
        ASSERT_TRUE(bRet == sal_True && sString.equals(rtl::OString("Tsch\xFC\xDF")) == sal_True) << "Strings must be equal";
    }

} // namespace rtl_string


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
