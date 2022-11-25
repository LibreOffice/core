/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppu/unotype.hxx>
#include <o3tl/any.hxx>

using namespace com::sun::star;

namespace
{
class MakePropertyValueTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(MakePropertyValueTest);
    CPPUNIT_TEST(testLvalue);
    CPPUNIT_TEST(testRvalue);
    CPPUNIT_TEST(testBitField);
    CPPUNIT_TEST(testJson);
    CPPUNIT_TEST_SUITE_END();

    void testLvalue()
    {
        sal_Int32 const i = 123;
        auto const v = comphelper::makePropertyValue("test", i);
        CPPUNIT_ASSERT_EQUAL(cppu::UnoType<sal_Int32>::get(), v.Value.getValueType());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(123), *o3tl::doAccess<sal_Int32>(v.Value));
    }

    void testRvalue()
    {
        auto const v = comphelper::makePropertyValue("test", sal_Int32(456));
        CPPUNIT_ASSERT_EQUAL(cppu::UnoType<sal_Int32>::get(), v.Value.getValueType());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(456), *o3tl::doAccess<sal_Int32>(v.Value));
    }

    void testBitField()
    {
        struct
        {
            bool b : 1;
        } s = { false };
        auto const v = comphelper::makePropertyValue("test", s.b);
        CPPUNIT_ASSERT_EQUAL(cppu::UnoType<bool>::get(), v.Value.getValueType());
        CPPUNIT_ASSERT_EQUAL(false, *o3tl::doAccess<bool>(v.Value));
    }

    void testJson()
    {
        std::vector<beans::PropertyValue> aRet = comphelper::JsonToPropertyValues(R"json(
{
    "FieldType": {
        "type": "string",
        "value": "vnd.oasis.opendocument.field.UNHANDLED"
    },
    "FieldCommandPrefix": {
        "type": "string",
        "value": "ADDIN ZOTERO_ITEM"
    },
    "Fields": {
        "type": "[][]com.sun.star.beans.PropertyValue",
        "value": [
            {
                "FieldType": {
                    "type": "string",
                    "value": "vnd.oasis.opendocument.field.UNHANDLED"
                },
                "FieldCommand": {
                    "type": "string",
                    "value": "ADDIN ZOTERO_ITEM new command 1"
                },
                "Fields": {
                    "type": "string",
                    "value": "new result 1"
                }
            },
            {
                "FieldType": {
                    "type": "string",
                    "value": "vnd.oasis.opendocument.field.UNHANDLED"
                },
                "FieldCommandPrefix": {
                    "type": "string",
                    "value": "ADDIN ZOTERO_ITEM new command 2"
                },
                "Fields": {
                    "type": "string",
                    "value": "new result 2"
                }
            }
        ]
    }
}
)json");
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aRet.size());
        beans::PropertyValue aFirst = aRet[0];
        CPPUNIT_ASSERT_EQUAL(OUString("FieldType"), aFirst.Name);
        CPPUNIT_ASSERT_EQUAL(OUString("vnd.oasis.opendocument.field.UNHANDLED"),
                             aFirst.Value.get<OUString>());
        beans::PropertyValue aSecond = aRet[1];
        CPPUNIT_ASSERT_EQUAL(OUString("FieldCommandPrefix"), aSecond.Name);
        CPPUNIT_ASSERT_EQUAL(OUString("ADDIN ZOTERO_ITEM"), aSecond.Value.get<OUString>());
        beans::PropertyValue aThird = aRet[2];
        CPPUNIT_ASSERT_EQUAL(OUString("Fields"), aThird.Name);
        uno::Sequence<uno::Sequence<beans::PropertyValue>> aSeqs;
        aThird.Value >>= aSeqs;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aSeqs.getLength());
        uno::Sequence<beans::PropertyValue> aFirstSeq = aSeqs[0];
        CPPUNIT_ASSERT_EQUAL(OUString("FieldType"), aFirstSeq[0].Name);
        CPPUNIT_ASSERT_EQUAL(OUString("FieldCommand"), aFirstSeq[1].Name);
        CPPUNIT_ASSERT_EQUAL(OUString("ADDIN ZOTERO_ITEM new command 1"),
                             aFirstSeq[1].Value.get<OUString>());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MakePropertyValueTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
