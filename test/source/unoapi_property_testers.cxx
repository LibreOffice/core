/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void testBooleanProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                         const OUString& name)
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

void testBooleanOptionalProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                                 const OUString& rName)
{
    try
    {
        testBooleanProperty(xPropertySet, rName);
    }
    catch (const css::beans::UnknownPropertyException& /*ex*/)
    {
        // ignore if the property is unknown as it is optional
    }
}

void testBooleanReadonlyProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                                 const OUString& name)
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
    OString msgSet = "Able to set PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgSet.getStr(), bPropertyGet, bPropertySet);
}

void testDoubleProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                        const OUString& name, const double& dValue)
{
    uno::Any aNewValue;

    double dPropertyGet;
    double dPropertySet;

    OString msgGet
        = "Unable to get PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_MESSAGE(msgGet.getStr(), xPropertySet->getPropertyValue(name) >>= dPropertyGet);

    aNewValue <<= dValue;
    xPropertySet->setPropertyValue(name, aNewValue);
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(name) >>= dPropertySet);
    OString msgSet
        = "Unable to set PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(msgSet.getStr(), dValue, dPropertySet, 0.5);
}

void testDoubleReadonlyProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                                const OUString& name, const double& dValue)
{
    uno::Any aNewValue;

    double dPropertyGet;
    double dPropertySet;

    OString msgGet
        = "Unable to get PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_MESSAGE(msgGet.getStr(), xPropertySet->getPropertyValue(name) >>= dPropertyGet);

    aNewValue <<= dValue;
    xPropertySet->setPropertyValue(name, aNewValue);
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(name) >>= dPropertySet);
    OString msgSet = "Able to set PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(msgSet.getStr(), dPropertyGet, dPropertySet, 0.5);
}

void testLongProperty(uno::Reference<beans::XPropertySet> const& xPropertySet, const OUString& name,
                      const sal_Int32& nValue)
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

void testLongOptionalProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                              const OUString& rName, const sal_Int32& rValue)
{
    try
    {
        testLongProperty(xPropertySet, rName, rValue);
    }
    catch (const css::beans::UnknownPropertyException& /*ex*/)
    {
        // ignore if the property is unknown as it is optional
    }
}

void testLongReadonlyProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                              const OUString& name, const sal_Int32& nValue)
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
    OString msgSet = "Able to set PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgSet.getStr(), nPropertyGet, nPropertySet);
}

void testShortProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                       const OUString& name, const sal_Int16& nValue)
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

void testShortOptionalProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                               const OUString& rName, const sal_Int16& rValue)
{
    try
    {
        testShortProperty(xPropertySet, rName, rValue);
    }
    catch (const css::beans::UnknownPropertyException& /*ex*/)
    {
        // ignore if the property is unknown as it is optional
    }
}

void testShortReadonlyProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                               const OUString& name, const sal_Int16& nValue)
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
    OString msgSet = "Able to set PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgSet.getStr(), nPropertyGet, nPropertySet);
}

void testStringOptionalProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                                const OUString& rName, const OUString& rValue)
{
    try
    {
        testStringProperty(xPropertySet, rName, rValue);
    }
    catch (const css::beans::UnknownPropertyException& /*ex*/)
    {
        // ignore if the property is unknown as it is optional
    }
}

void testStringProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                        const OUString& name, const OUString& rValue)
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

void testStringReadonlyProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                                const OUString& name, const OUString& rValue)
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
    OString msgSet = "Able to set PropertyValue: " + OUStringToOString(name, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgSet.getStr(), sPropertyGet, sPropertySet);
}

void testColorProperty(uno::Reference<beans::XPropertySet> const& xPropertySet,
                       const OUString& name, const util::Color& rValue)
{
    uno::Any aNewValue;

    util::Color sPropertyGet;
    util::Color sPropertySet;

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

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
