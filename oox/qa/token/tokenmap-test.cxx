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

#include "oox/token/tokenmap.hxx"
#include "oox/token/tokens.hxx"

using namespace std;
using namespace com::sun::star::uno;

namespace oox {

class TokenmapTest: public CppUnit::TestFixture
{
public:
    void test_roundTrip();

    CPPUNIT_TEST_SUITE(TokenmapTest);

    CPPUNIT_TEST(test_roundTrip);
    CPPUNIT_TEST_SUITE_END();

private:
    TokenMap tokenMap;
};

void TokenmapTest::test_roundTrip()
{
    for ( sal_Int32 nToken = 0; nToken < XML_TOKEN_COUNT; ++nToken )
    {
        // check that the getIdentifier <-> getToken roundtrip works
        Sequence< sal_Int8 > rUtf8Name = tokenMap.getUtf8TokenName(nToken);
        sal_Int32 ret = tokenMap.getTokenFromUTF8(
            reinterpret_cast< const char * >(rUtf8Name.getConstArray()),
            rUtf8Name.getLength() );
        CPPUNIT_ASSERT_EQUAL(ret, nToken);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(TokenmapTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();
