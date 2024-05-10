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
    xAttributeList->add(2, OString::Concat("2"));
    xAttributeList->add(3, u"3");
    xAttributeList->add(4, OUString::Concat("4"));

    // We can't test getValueToken() and getOptionalValueToken()
    // without XFastTokenHandler :-(
    // Uncomment to get segmentation fault:
    // xAttributeList->getOptionalValueToken(1, 0);
    // xAttributeList->getValueToken(2);

    CPPUNIT_ASSERT( xAttributeList->hasAttribute(1) );
    CPPUNIT_ASSERT( !xAttributeList->hasAttribute(5) );

    CPPUNIT_ASSERT_EQUAL( u"2"_ustr, xAttributeList->getOptionalValue(2)  );
    CPPUNIT_ASSERT_EQUAL( u"3"_ustr, xAttributeList->getOptionalValue(3) );
    CPPUNIT_ASSERT_EQUAL( u"4"_ustr, xAttributeList->getOptionalValue(4) );
    CPPUNIT_ASSERT_EQUAL( OUString(), xAttributeList->getOptionalValue(5) );

    CPPUNIT_ASSERT_EQUAL( u"1"_ustr, xAttributeList->getValue(1) );
    CPPUNIT_ASSERT_THROW( xAttributeList->getValue(5), xml::sax::SAXException );

    xAttributeList->addUnknown("a"_ostr, "a"_ostr);
    xAttributeList->addUnknown(u"b"_ustr, "b"_ostr, "b"_ostr);
    xAttributeList->addUnknown("c"_ostr, "c"_ostr);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(3), xAttributeList->getUnknownAttributes().getLength() );

    CPPUNIT_ASSERT_EQUAL( sal_Int32(4), xAttributeList->getFastAttributes().getLength() );

    xAttributeList->clear();
    CPPUNIT_ASSERT( !xAttributeList->hasAttribute(1) );
    CPPUNIT_ASSERT( !xAttributeList->getFastAttributes().hasElements() );
    xAttributeList->addUnknown("c"_ostr, "c"_ostr);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(1), xAttributeList->getUnknownAttributes().getLength() );
}

CPPUNIT_TEST_SUITE_REGISTRATION( AttributesTest );

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
