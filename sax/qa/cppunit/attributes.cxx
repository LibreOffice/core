/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sax/fastattribs.hxx>

using namespace css;
using namespace css::xml;

namespace {

class AttributesTest: public CppUnit::TestFixture
{
    bool mbException;

public:
    void test();

    CPPUNIT_TEST_SUITE( AttributesTest );
    CPPUNIT_TEST( test );
    CPPUNIT_TEST_SUITE_END();
};

void AttributesTest::test()
{
    sax_fastparser::FastAttributeList aAttributeList( NULL );
    aAttributeList.add(1, "1");
    aAttributeList.add(2, OString("2"));

    // We can't test getValueToken() and getOptionalValueToken()
    // without XFastTokenHandler :-(
    // Uncomment to get segmantation fault:
    // aAttributeList.getOptionalValueToken(1, 0);
    // aAttributeList.getValueToken(2);

    CPPUNIT_ASSERT( aAttributeList.hasAttribute(1) );
    CPPUNIT_ASSERT( !aAttributeList.hasAttribute(3) );

    CPPUNIT_ASSERT_EQUAL( aAttributeList.getOptionalValue(2), OUString("2") );
    CPPUNIT_ASSERT_EQUAL( aAttributeList.getOptionalValue(3), OUString() );

    CPPUNIT_ASSERT_EQUAL( aAttributeList.getValue(1), OUString("1") );
    mbException = false;

    try { aAttributeList.getValue(3); }
    catch (const sax::SAXException& )
    {
        mbException = true;
    }
    CPPUNIT_ASSERT( mbException );

    aAttributeList.addUnknown("a", "a");
    aAttributeList.addUnknown("b", "b", "b");
    aAttributeList.addUnknown("c", "c");
    CPPUNIT_ASSERT_EQUAL( 3, aAttributeList.getUnknownAttributes().getLength() );

    CPPUNIT_ASSERT_EQUAL( 2, aAttributeList.getFastAttributes().getLength() );

    aAttributeList.clear();
    CPPUNIT_ASSERT( !aAttributeList.hasAttribute(1) );
    CPPUNIT_ASSERT_EQUAL( 0, aAttributeList.getFastAttributes().getLength() );
    aAttributeList.addUnknown("c", "c");
    CPPUNIT_ASSERT_EQUAL( 1, aAttributeList.getUnknownAttributes().getLength() );
}

CPPUNIT_TEST_SUITE_REGISTRATION( AttributesTest );

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
