/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>

#include <iostream>

namespace ScAutoFormatFieldObj {

class ScTableAutoFormatField : public UnoApiTest
{
public:

    uno::Reference< beans::XPropertySet > init();    void testRotateReference();
    void testVertJustify();

    CPPUNIT_TEST_SUITE(ScTableAutoFormatField);
    // both tests are broken
    // related to: 7da57d17b6179e71c8b6d7549ad89eaf3a4a28c6
    //CPPUNIT_TEST(testRotateReference);
    //CPPUNIT_TEST(testVertJustify);
    CPPUNIT_TEST_SUITE_END();

};

uno::Reference< beans::XPropertySet > ScTableAutoFormatField::init()
{
    uno::Reference< container::XIndexAccess > xAutoFormatFields( m_xSFactory->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.TableAutoFormats"))), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xAutoFormatFields.is());
    uno::Reference< container::XIndexAccess > xIndex( xAutoFormatFields->getByIndex(0), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xIndex.is());
    uno::Reference< beans::XPropertySet > xReturn( xIndex->getByIndex(0), UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE("could not create table auto formats", xReturn.is());
    return xReturn;
}

void ScTableAutoFormatField::testRotateReference()
{
    uno::Reference< beans::XPropertySet > xTableAutoFormatField = init();
    rtl::OUString aRotateReference(RTL_CONSTASCII_USTRINGPARAM("RotateReference"));
    uno::Any aOldRotateReference = xTableAutoFormatField->getPropertyValue(aRotateReference);
    sal_Int32 aValue;
    CPPUNIT_ASSERT(aOldRotateReference >>= aValue);
    std::cout << "Old RotateReferene Value: " << aValue << std::endl;

    uno::Any aNewRotateReference;
    aNewRotateReference <<= table::CellVertJustify_BOTTOM;
    xTableAutoFormatField->setPropertyValue(aRotateReference, aNewRotateReference);
    uno::Any aRotateReferenceControllValue = xTableAutoFormatField->getPropertyValue(aRotateReference);
    CPPUNIT_ASSERT(aRotateReferenceControllValue >>= aValue);
    std::cout << "New VertJustify value: " << aValue << std::endl;
    CPPUNIT_ASSERT_MESSAGE("value has not been changed", aValue == table::CellVertJustify_BOTTOM);
}

void ScTableAutoFormatField::testVertJustify()
{
    uno::Reference< beans::XPropertySet > xTableAutoFormatField = init();
    rtl::OUString aVertJustify(RTL_CONSTASCII_USTRINGPARAM("VertJustify"));
    uno::Any aOldVertJustify = xTableAutoFormatField->getPropertyValue(aVertJustify);
    sal_Int32 aValue;
    CPPUNIT_ASSERT(aOldVertJustify >>= aValue);
    std::cout << "Old VertJustify value: " << aValue << std::endl;

    uno::Any aNewVertJustify;
    aNewVertJustify <<= table::CellVertJustify_BOTTOM;
    xTableAutoFormatField->setPropertyValue(aVertJustify, aNewVertJustify);
    uno::Any aVertJustifyControllValue = xTableAutoFormatField->getPropertyValue(aVertJustify);
    CPPUNIT_ASSERT(aVertJustifyControllValue >>= aValue);
    std::cout << "New VertJustify value: " << aValue << std::endl;
    CPPUNIT_ASSERT_MESSAGE("value has not been changed", aValue == table::CellVertJustify_BOTTOM);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableAutoFormatField);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
