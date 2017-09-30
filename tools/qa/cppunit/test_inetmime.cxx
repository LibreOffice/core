/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include <tools/inetmime.hxx>

namespace
{

    class Test: public CppUnit::TestFixture
    {
        bool testDecode(char const * input, char const * expected);
    public:
        void test_decodeHeaderFieldBody();

        void test_scanContentType_basic();
        void test_scanContentType_rfc2231();

        CPPUNIT_TEST_SUITE(Test);
        CPPUNIT_TEST(test_decodeHeaderFieldBody);
        CPPUNIT_TEST(test_scanContentType_basic);
        CPPUNIT_TEST(test_scanContentType_rfc2231);
        CPPUNIT_TEST_SUITE_END();
    };

    bool Test::testDecode(char const * input, char const * expected)
    {
        OUString result = INetMIME::decodeHeaderFieldBody(input);
        return result.equalsAscii(expected);
    }

    void Test::test_decodeHeaderFieldBody()
    {
        CPPUNIT_ASSERT(testDecode("=?iso-8859-1?B?QQ==?=", "A"));
        CPPUNIT_ASSERT(testDecode("=?iso-8859-1?B?QUI=?=", "AB"));
        CPPUNIT_ASSERT(testDecode("=?iso-8859-1?B?QUJD?=", "ABC"));
    }

    void Test::test_scanContentType_basic()
    {
        OUString input
            = "TEST/subTST; parm1=Value1; Parm2=\"unpacked value; %20\"";
        // Just scan input for valid string:
        auto end = INetMIME::scanContentType(input);
        CPPUNIT_ASSERT(end != nullptr);
        CPPUNIT_ASSERT_EQUAL(OUString(), OUString(end));
        // Scan input and parse type, subType and parameters:
        OUString type;
        OUString subType;
        INetContentTypeParameterList parameters;
        end = INetMIME::scanContentType(input,
                                        &type, &subType, &parameters);
        CPPUNIT_ASSERT(end != nullptr);
        CPPUNIT_ASSERT_EQUAL(OUString(), OUString(end));
        CPPUNIT_ASSERT_EQUAL(OUString("test"), type);
        CPPUNIT_ASSERT_EQUAL(OUString("subtst"), subType);
        CPPUNIT_ASSERT_EQUAL(
            INetContentTypeParameterList::size_type(2), parameters.size());
        auto i = parameters.find("parm1");
        CPPUNIT_ASSERT(i != parameters.end());
        CPPUNIT_ASSERT_EQUAL(OString(), i->second.m_sCharset);
        CPPUNIT_ASSERT_EQUAL(OString(), i->second.m_sLanguage);
        CPPUNIT_ASSERT_EQUAL(OUString("Value1"), i->second.m_sValue);
        CPPUNIT_ASSERT(i->second.m_bConverted);
        i = parameters.find("parm2");
        CPPUNIT_ASSERT(i != parameters.end());
        CPPUNIT_ASSERT_EQUAL(OString(), i->second.m_sCharset);
        CPPUNIT_ASSERT_EQUAL(OString(), i->second.m_sLanguage);
        CPPUNIT_ASSERT_EQUAL(OUString("unpacked value; %20"), i->second.m_sValue);
        CPPUNIT_ASSERT(i->second.m_bConverted);
    }

    void Test::test_scanContentType_rfc2231()
    {
        // Test extended parameter with value split in 3 sections:
        OUString input
            = "TEST/subTST; "
              "parm1*0*=US-ASCII'En'5%25%20; "
              "Parm1*1*=of%2010;\t"
              "parm1*2*=%20%3d%200.5";
        // Just scan input for valid string:
        auto end = INetMIME::scanContentType(input);
        CPPUNIT_ASSERT(end != nullptr);
        CPPUNIT_ASSERT_EQUAL(OUString(), OUString(end));
        // Scan input and parse type, subType and parameters:
        OUString type;
        OUString subType;
        INetContentTypeParameterList parameters;
        end = INetMIME::scanContentType(input,
                                        &type, &subType, &parameters);
        CPPUNIT_ASSERT(end != nullptr);
        CPPUNIT_ASSERT_EQUAL(OUString(), OUString(end));
        CPPUNIT_ASSERT_EQUAL(OUString("test"), type);
        CPPUNIT_ASSERT_EQUAL(OUString("subtst"), subType);
        CPPUNIT_ASSERT_EQUAL(
            INetContentTypeParameterList::size_type(1), parameters.size());
        auto i = parameters.find("parm1");
        CPPUNIT_ASSERT(i != parameters.end());
        CPPUNIT_ASSERT_EQUAL(OString("us-ascii"), i->second.m_sCharset);
        CPPUNIT_ASSERT_EQUAL(OString("en"), i->second.m_sLanguage);
        CPPUNIT_ASSERT_EQUAL(OUString("5% of 10 = 0.5"), i->second.m_sValue);
        CPPUNIT_ASSERT(i->second.m_bConverted);

        // Test extended parameters with different value charsets:
        input = "TEST/subTST;"
                "parm1*0*=us-ascii'en'value;PARM1*1*=1;"
                "parm2*0*=WINDOWS-1250'en-GB'value2%20%80;"
                "parm3*0*=UNKNOWN'EN'value3;"
                "parm1*1*=2";               // this parameter is a duplicate,
                                            // the scan should end before this parameter
        // Just scan input for valid string:
        end = INetMIME::scanContentType(input);
        CPPUNIT_ASSERT(end != nullptr);
        CPPUNIT_ASSERT_EQUAL(OUString(";parm1*1*=2"), OUString(end)); // the invalid end of input
        // Scan input and parse type, subType and parameters:
        end = INetMIME::scanContentType(input,
                                        &type, &subType, &parameters);
        CPPUNIT_ASSERT(end != nullptr);
        CPPUNIT_ASSERT_EQUAL(OUString(";parm1*1*=2"), OUString(end)); // the invalid end of input
        CPPUNIT_ASSERT_EQUAL(OUString("test"), type);
        CPPUNIT_ASSERT_EQUAL(OUString("subtst"), subType);
        CPPUNIT_ASSERT_EQUAL(
            INetContentTypeParameterList::size_type(3), parameters.size());
        i = parameters.find("parm1");
        CPPUNIT_ASSERT(i != parameters.end());
        CPPUNIT_ASSERT_EQUAL(OString("us-ascii"), i->second.m_sCharset);
        CPPUNIT_ASSERT_EQUAL(OString("en"), i->second.m_sLanguage);
        CPPUNIT_ASSERT_EQUAL(OUString("value1"), i->second.m_sValue);
        CPPUNIT_ASSERT(i->second.m_bConverted);
        i = parameters.find("parm2");
        CPPUNIT_ASSERT(i != parameters.end());
        CPPUNIT_ASSERT_EQUAL(OString("windows-1250"), i->second.m_sCharset);
        CPPUNIT_ASSERT_EQUAL(OString("en-gb"), i->second.m_sLanguage);
        // Euro currency sign, windows-1250 x80 is converted to unicode u20AC:
        CPPUNIT_ASSERT_EQUAL(OUString(u"value2 \u20AC"), i->second.m_sValue);
        CPPUNIT_ASSERT(i->second.m_bConverted);
        i = parameters.find("parm3");
        CPPUNIT_ASSERT(i != parameters.end());
        CPPUNIT_ASSERT_EQUAL(OString("unknown"), i->second.m_sCharset);
        CPPUNIT_ASSERT_EQUAL(OString("en"), i->second.m_sLanguage);
        // Conversion fails for unknown charsets:
        CPPUNIT_ASSERT(!i->second.m_bConverted);
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
