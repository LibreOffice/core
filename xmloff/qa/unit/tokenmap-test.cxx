/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "xmloff/fasttokenhandler.hxx"
#include "xmloff/token/tokens.hxx"

using namespace std;
using namespace com::sun::star::uno;

namespace xmloff {

class TokenmapTest: public CppUnit::TestFixture
{
public:

    TokenmapTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void test_roundTrip();

    CPPUNIT_TEST_SUITE(TokenmapTest);

    CPPUNIT_TEST(test_roundTrip);
    CPPUNIT_TEST_SUITE_END();

private:
    token::TokenMap* pTokenMap;
};

TokenmapTest::TokenmapTest() : pTokenMap(nullptr)
{
}

void TokenmapTest::setUp()
{
    CppUnit::TestFixture::setUp();
    pTokenMap = new token::TokenMap;
}

void TokenmapTest::tearDown()
{
    delete pTokenMap;
    CppUnit::TestFixture::tearDown();
}

void TokenmapTest::test_roundTrip()
{
    for ( sal_Int32 nToken = 0; nToken < XML_TOKEN_COUNT; ++nToken )
    {
        // check that the getIdentifier <-> getToken roundtrip works
        Sequence< sal_Int8 > rUtf8Name = pTokenMap->getUtf8TokenName(nToken);
        CPPUNIT_ASSERT_MESSAGE("Token name sequence should not be empty", rUtf8Name.getLength());
        const char* pChar = reinterpret_cast< const char * >(rUtf8Name.getConstArray());
        CPPUNIT_ASSERT_MESSAGE("Token name sequence array pointer failed", pChar);
        sal_Int32 ret = token::TokenMap::getTokenFromUTF8( pChar, rUtf8Name.getLength() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("No roundtrip for token", ret, nToken);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(TokenmapTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();
