/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/ref.hxx>
#include <sax/fastattribs.hxx>

using namespace css;
using namespace css::xml;

namespace {

class AttributesTest: public CppUnit::TestFixture
{
public:
    void test();

    CPPUNIT_TEST_SUITE( AttributesTest );
    CPPUNIT_TEST( test );
    CPPUNIT_TEST_SUITE_END();
};

void AttributesTest::test()
{
    rtl::Reference<sax_fastparser::FastAttributeList> xAttributeList( new sax_fastparser::FastAttributeList(nullptr) );
    xAttributeList->add(1, "1");
    xAttributeList->add(2, OString("2"));

    // We can't test getValueToken() and getOptionalValueToken()
    // without XFastTokenHandler :-(
    // Uncomment to get segmantation fault:
    // xAttributeList->getOptionalValueToken(1, 0);
    // xAttributeList->getValueToken(2);

    CPPUNIT_ASSERT( xAttributeList->hasAttribute(1) );
    CPPUNIT_ASSERT( !xAttributeList->hasAttribute(3) );

    CPPUNIT_ASSERT_EQUAL( xAttributeList->getOptionalValue(2), OUString("2") );
    CPPUNIT_ASSERT_EQUAL( xAttributeList->getOptionalValue(3), OUString() );

    CPPUNIT_ASSERT_EQUAL( xAttributeList->getValue(1), OUString("1") );
    CPPUNIT_ASSERT_THROW( xAttributeList->getValue(3), xml::sax::SAXException );

    xAttributeList->addUnknown("a", "a");
    xAttributeList->addUnknown("b", "b", "b");
    xAttributeList->addUnknown("c", "c");
    CPPUNIT_ASSERT_EQUAL( (sal_Int32) 3, xAttributeList->getUnknownAttributes().getLength() );

    CPPUNIT_ASSERT_EQUAL( (sal_Int32) 2, xAttributeList->getFastAttributes().getLength() );

    xAttributeList->clear();
    CPPUNIT_ASSERT( !xAttributeList->hasAttribute(1) );
    CPPUNIT_ASSERT_EQUAL( (sal_Int32) 0, xAttributeList->getFastAttributes().getLength() );
    xAttributeList->addUnknown("c", "c");
    CPPUNIT_ASSERT_EQUAL( (sal_Int32) 1, xAttributeList->getUnknownAttributes().getLength() );
}

CPPUNIT_TEST_SUITE_REGISTRATION( AttributesTest );

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
