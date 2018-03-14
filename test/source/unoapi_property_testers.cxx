/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void testBooleanProperty(uno::Reference<beans::XPropertySet>& xPropertySet, const OUString& name)
{
    uno::Any aNewValue;

    bool bPropertyGet = false;
    bool bPropertySet = false;

    OString msgGet
        = "Unable to get PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_MESSAGE(msgGet.getStr(), xPropertySet->getPropertyValue(name) >>= bPropertyGet);

    aNewValue <<= !bPropertyGet;
    xPropertySet->setPropertyValue(name, aNewValue);
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(name) >>= bPropertySet);
    OString msgSet
        = "Unable to set PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgSet.getStr(), !bPropertyGet, bPropertySet);
}

void testLongProperty(uno::Reference<beans::XPropertySet>& xPropertySet, const OUString& name,
                      const sal_Int32& nValue = 42)
{
    uno::Any aNewValue;

    sal_Int32 nPropertyGet;
    sal_Int32 nPropertySet;

    OString msgGet
        = "Unable to get PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_MESSAGE(msgGet.getStr(), xPropertySet->getPropertyValue(name) >>= nPropertyGet);

    aNewValue <<= nValue;
    xPropertySet->setPropertyValue(name, aNewValue);
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(name) >>= nPropertySet);
    OString msgSet
        = "Unable to set PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgSet.getStr(), nValue, nPropertySet);
}

void testShortProperty(uno::Reference<beans::XPropertySet>& xPropertySet, const OUString& name,
                       const sal_Int16& nValue = 42)
{
    uno::Any aNewValue;

    sal_Int16 nPropertyGet;
    sal_Int16 nPropertySet;

    OString msgGet
        = "Unable to get PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_MESSAGE(msgGet.getStr(), xPropertySet->getPropertyValue(name) >>= nPropertyGet);

    aNewValue <<= nValue;
    xPropertySet->setPropertyValue(name, aNewValue);
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(name) >>= nPropertySet);
    OString msgSet
        = "Unable to set PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgSet.getStr(), nValue, nPropertySet);
}

void testStringProperty(uno::Reference<beans::XPropertySet>& xPropertySet, const OUString& name,
                        const OUString& rValue)
{
    uno::Any aNewValue;

    OUString sPropertyGet;
    OUString sPropertySet;

    OString msgGet
        = "Unable to get PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_MESSAGE(msgGet.getStr(), xPropertySet->getPropertyValue(name) >>= sPropertyGet);

    aNewValue <<= rValue;
    xPropertySet->setPropertyValue(name, aNewValue);
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(name) >>= sPropertySet);
    OString msgSet
        = "Unable to set PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgSet.getStr(), rValue, sPropertySet);
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
