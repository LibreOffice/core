/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "xmloff/fasttokenhandler.hxx"
#include "xmloff/token/tokens.hxx"
#include <xmloff/xmltoken.hxx>

using namespace std;
using namespace com::sun::star::uno;

namespace xmloff {

class TokenmapTest: public CppUnit::TestFixture
{
public:

    TokenmapTest();

    void test_roundTrip();
    void test_listEquality();

    CPPUNIT_TEST_SUITE(TokenmapTest);

    CPPUNIT_TEST(test_roundTrip);
    CPPUNIT_TEST(test_listEquality);

    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<token::TokenMap> pTokenMap;
};

TokenmapTest::TokenmapTest() : pTokenMap(new token::TokenMap)
{
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

void TokenmapTest::test_listEquality()
{
    //make sure the two token lists stay in sync
    for ( sal_Int32 nToken = 0; nToken < XML_TOKEN_COUNT; ++nToken )
    {
        Sequence< sal_Int8 > rUtf8Name = pTokenMap->getUtf8TokenName(nToken);
        const OUString& rName = OUString( reinterpret_cast< const char* >(
                        rUtf8Name.getConstArray() ), rUtf8Name.getLength(), RTL_TEXTENCODING_UTF8 );
        if ( rName.endsWith("_DUMMY") )
            continue;
        const OUString& rTokenName = GetXMLToken( static_cast<xmloff::token::XMLTokenEnum>(nToken) );
        CPPUNIT_ASSERT_EQUAL(rName, rTokenName);
    }

    for ( sal_Int32 nToken = xmloff::token::XMLTokenEnum::XML_TOKEN_START + 1;
            nToken < xmloff::token::XMLTokenEnum::XML_TOKEN_END; ++nToken )
    {
        const OUString& rTokenName = GetXMLToken( static_cast<xmloff::token::XMLTokenEnum>(nToken) );
        Sequence< sal_Int8 > rUtf8Name = pTokenMap->getUtf8TokenName(nToken);
        const OUString& rName = OUString( reinterpret_cast< const char* >(
                        rUtf8Name.getConstArray() ), rUtf8Name.getLength(), RTL_TEXTENCODING_UTF8 );
        if ( !rName.endsWith("_DUMMY") )
            CPPUNIT_ASSERT_EQUAL(rTokenName, rName);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(TokenmapTest);

}

