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

        void test_scanContentType();

        CPPUNIT_TEST_SUITE(Test);
        CPPUNIT_TEST(test_decodeHeaderFieldBody);
        CPPUNIT_TEST(test_scanContentType);
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

    void Test::test_scanContentType()
    {
        {
            OUString input
                = "TEST/subTST; parm1*0*=US-ASCII'En'5%25%20; Parm1*1*=of%2010";
            OUString type;
            OUString subType;
            INetContentTypeParameterList parameters;
            auto end = INetMIME::scanContentType(
                input.getStr(), input.getStr() + input.getLength(), &type,
                &subType, &parameters);
            CPPUNIT_ASSERT(end != nullptr);
            CPPUNIT_ASSERT_EQUAL(OUString(), OUString(end));
            CPPUNIT_ASSERT_EQUAL(OUString("test"), type);
            CPPUNIT_ASSERT_EQUAL(OUString("subtst"), subType);
            CPPUNIT_ASSERT_EQUAL(
                INetContentTypeParameterList::size_type(1), parameters.size());
            auto i = parameters.find("parm1");
            CPPUNIT_ASSERT(i != parameters.end());
            CPPUNIT_ASSERT_EQUAL(OUString("5% of 10"), i->second.m_sValue);
        }
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
