/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tox.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class ToxMiscTest : public CppUnit::TestFixture
{
public:
    void testTdf73162();
    CPPUNIT_TEST_SUITE(ToxMiscTest);
    CPPUNIT_TEST(testTdf73162);
    CPPUNIT_TEST_SUITE_END();
};

void ToxMiscTest::testTdf73162()
{
    //create Tokens with the help of input string
    SwFormTokensHelper tokensHelper(
        u"<LS Index Link,65535,><E# ,65535,0,10><X ,65535,\001.\001><ET ,65535,><T "
        ",65535,0,5,.,1><# ,65535,><LE ,65535,>"); //input token string
    SwFormTokens formTokens = tokensHelper.GetTokens();

    //check the size of token vector / count of tokens
    CPPUNIT_ASSERT_EQUAL(std::vector<SwFormToken>::size_type(7), formTokens.size());

    //check individual tokens for proper values
    //first token
    SwFormToken token1 = formTokens.at(0);
    CPPUNIT_ASSERT_EQUAL(FormTokenType::TOKEN_LINK_START, token1.eTokenType);
    CPPUNIT_ASSERT_EQUAL(OUString("<LS Index Link,65535,>"), token1.GetString());
    //second token
    SwFormToken token2 = formTokens.at(1);
    CPPUNIT_ASSERT_EQUAL(FormTokenType::TOKEN_ENTRY_NO, token2.eTokenType);
    CPPUNIT_ASSERT_EQUAL(OUString("<E# ,65535,0,10>"), token2.GetString());
    //third token
    SwFormToken token3 = formTokens.at(2);
    CPPUNIT_ASSERT_EQUAL(FormTokenType::TOKEN_TEXT, token3.eTokenType);
    CPPUNIT_ASSERT_EQUAL(OUString("<X ,65535,\001.\001>"), token3.GetString());
    //fourth token
    SwFormToken token4 = formTokens.at(3);
    CPPUNIT_ASSERT_EQUAL(FormTokenType::TOKEN_ENTRY_TEXT, token4.eTokenType);
    CPPUNIT_ASSERT_EQUAL(OUString("<ET ,65535,>"), token4.GetString());
    //fifth token
    SwFormToken token5 = formTokens.at(4);
    CPPUNIT_ASSERT_EQUAL(FormTokenType::TOKEN_TAB_STOP, token5.eTokenType);
    CPPUNIT_ASSERT_EQUAL(OUString("<T ,65535,0,5,.,1>"), token5.GetString());
    //sixth token
    SwFormToken token6 = formTokens.at(5);
    CPPUNIT_ASSERT_EQUAL(FormTokenType::TOKEN_PAGE_NUMS, token6.eTokenType);
    CPPUNIT_ASSERT_EQUAL(OUString("<# ,65535,>"), token6.GetString());
    //seventh token
    SwFormToken token7 = formTokens.at(6);
    CPPUNIT_ASSERT_EQUAL(FormTokenType::TOKEN_LINK_END, token7.eTokenType);
    CPPUNIT_ASSERT_EQUAL(OUString("<LE ,65535,>"), token7.GetString());
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(ToxMiscTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
