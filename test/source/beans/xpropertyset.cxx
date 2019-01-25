/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/beans/xpropertyset.hxx>

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Type.h>

#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include <set>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
XPropertySet::PropsToTest::PropsToTest()
    : initialized(false)
{
}

class MockedPropertyChangeListener : public ::cppu::WeakImplHelper<beans::XPropertyChangeListener>
{
public:
    MockedPropertyChangeListener()
        : m_bListenerCalled(false)
    {
    }

    bool m_bListenerCalled;

    virtual void SAL_CALL propertyChange(const beans::PropertyChangeEvent& /* xEvent */) override
    {
        m_bListenerCalled = true;
    }

    virtual void SAL_CALL disposing(const lang::EventObject& /* xEventObj */) override {}
};

class MockedVetoableChangeListener : public ::cppu::WeakImplHelper<beans::XVetoableChangeListener>
{
public:
    MockedVetoableChangeListener()
        : m_bListenerCalled(false)
    {
    }

    bool m_bListenerCalled;

    virtual void SAL_CALL vetoableChange(const beans::PropertyChangeEvent& /* xEvent */) override
    {
        m_bListenerCalled = true;
    }

    virtual void SAL_CALL disposing(const lang::EventObject& /* xEventObj */) override {}
};

void XPropertySet::testPropertyChangeListener()
{
    uno::Reference<beans::XPropertySet> xPropSet(init(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySetInfo> xPropInfo = xPropSet->getPropertySetInfo();
    fillPropsToTest(xPropInfo);

    for (const auto& aName : maPropsToTest.bound)
    {
        rtl::Reference<MockedPropertyChangeListener> xListener = new MockedPropertyChangeListener();
        xPropSet->addPropertyChangeListener(
            aName, uno::Reference<beans::XPropertyChangeListener>(xListener.get()));
        if (!isPropertyValueChangeable(aName))
            continue;

        CPPUNIT_ASSERT(xListener->m_bListenerCalled);

        xListener->m_bListenerCalled = false;
        xPropSet->removePropertyChangeListener(
            aName, uno::Reference<beans::XPropertyChangeListener>(xListener.get()));
        isPropertyValueChangeable(aName);
        CPPUNIT_ASSERT(!xListener->m_bListenerCalled);
    }
}

void XPropertySet::testVetoableChangeListener()
{
    uno::Reference<beans::XPropertySet> xPropSet(init(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySetInfo> xPropInfo = xPropSet->getPropertySetInfo();
    fillPropsToTest(xPropInfo);

    for (const auto& aName : maPropsToTest.bound)
    {
        rtl::Reference<MockedVetoableChangeListener> xListener = new MockedVetoableChangeListener();
        xPropSet->addVetoableChangeListener(
            aName, uno::Reference<beans::XVetoableChangeListener>(xListener.get()));
        if (!isPropertyValueChangeable(aName))
            continue;

        CPPUNIT_ASSERT(xListener->m_bListenerCalled);

        xListener->m_bListenerCalled = false;
        xPropSet->removeVetoableChangeListener(
            aName, uno::Reference<beans::XVetoableChangeListener>(xListener.get()));
        isPropertyValueChangeable(aName);
        CPPUNIT_ASSERT(!xListener->m_bListenerCalled);
    }
}

void XPropertySet::testGetPropertySetInfo()
{
    uno::Reference<beans::XPropertySet> xPropSet(init(), UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySetInfo> xPropInfo = xPropSet->getPropertySetInfo();
    if (xPropInfo.is())
    {
        fillPropsToTest(xPropInfo);
    }
    else
    {
        // TODO: Add a means for the client code to populate the PropsToTest.
    }
}

void XPropertySet::testSetPropertyValue()
{
    testGetPropertySetInfo();

    for (size_t i = 0, n = maPropsToTest.normal.size(); i < n; ++i)
    {
        bool bSuccess = isPropertyValueChangeable(maPropsToTest.normal[i]);
        CPPUNIT_ASSERT(bSuccess);
    }
}

void XPropertySet::testGetPropertyValue()
{
    testGetPropertySetInfo();
    uno::Reference<beans::XPropertySet> xPropSet(init(), UNO_QUERY_THROW);

    // Check read-only properties.
    for (size_t i = 0, n = maPropsToTest.readonly.size(); i < n; ++i)
    {
        bool bSuccess = getSinglePropertyValue(xPropSet, maPropsToTest.readonly[i]);
        CPPUNIT_ASSERT(bSuccess);
    }

    // Check writable properties.
    for (size_t i = 0, n = maPropsToTest.normal.size(); i < n; ++i)
    {
        bool bSuccess = getSinglePropertyValue(xPropSet, maPropsToTest.normal[i]);
        CPPUNIT_ASSERT(bSuccess);
    }
}

bool XPropertySet::isPropertyValueChangeable(const OUString& rName)
{
    bool bIgnore = isPropertyIgnored(rName);
    if (bIgnore)
        return false;

    uno::Reference<beans::XPropertySet> xPropSet(init(), UNO_QUERY_THROW);
    try
    {
        uno::Any any = xPropSet->getPropertyValue(rName);
        const uno::Type& type = any.getValueType();
        if (type == cppu::UnoType<bool>::get())
        {
            // boolean type
            bool bOld = any.get<bool>();
            xPropSet->setPropertyValue(rName, makeAny(!bOld));
        }
        else if (type == cppu::UnoType<sal_Int8>::get())
        {
            // 8-bit integer
            sal_Int8 nOld = any.get<sal_Int8>();
            sal_Int8 nNew = nOld + 1;
            xPropSet->setPropertyValue(rName, makeAny(nNew));
        }
        else if (type == cppu::UnoType<sal_Int16>::get())
        {
            // 16-bit integer
            sal_Int16 nOld = any.get<sal_Int16>();
            sal_Int16 nNew = nOld + 1;
            xPropSet->setPropertyValue(rName, makeAny(nNew));
        }
        else if (type == cppu::UnoType<sal_Int32>::get())
        {
            // 32-bit integer
            sal_Int32 nOld = any.get<sal_Int32>();
            sal_Int32 nNew = nOld + 3;
            xPropSet->setPropertyValue(rName, makeAny(nNew));
        }
        else if (type == cppu::UnoType<sal_Int64>::get())
        {
            // 64-bit integer
            sal_Int64 nOld = any.get<sal_Int64>();
            sal_Int64 nNew = nOld + 4;
            xPropSet->setPropertyValue(rName, makeAny(nNew));
        }
        else if (type == cppu::UnoType<float>::get())
        {
            // single precision
            float fOld = any.get<float>();
            float fNew = fOld + 1.2;
            xPropSet->setPropertyValue(rName, makeAny(fNew));
        }
        else if (type == cppu::UnoType<double>::get())
        {
            // double precision
            double fOld = any.get<double>();
            double fNew = fOld + 1.3;
            xPropSet->setPropertyValue(rName, makeAny(fNew));
        }
        else if (type == cppu::UnoType<OUString>::get())
        {
            // string type
            OUString aOld = any.get<OUString>();
            OUString aNew = aOld + "foo";
            xPropSet->setPropertyValue(rName, makeAny(aNew));
        }
        else if (type == cppu::UnoType<util::DateTime>::get())
        {
            // date time type
            util::DateTime aDT = any.get<util::DateTime>();
            aDT.Year += 1;
            xPropSet->setPropertyValue(rName, makeAny(aDT));
        }
        else
        {
            std::cout << type.getTypeName() << std::endl;
            std::cout << rName << std::endl;
            CPPUNIT_ASSERT_MESSAGE(
                "XPropertySet::isPropertyValueChangeable: unknown type in Any tested.", false);
        }

        uno::Any anyTest = xPropSet->getPropertyValue(rName);
        return any != anyTest;
    }
    catch (const uno::Exception&)
    {
        CPPUNIT_ASSERT_MESSAGE("XPropertySet::isPropertyValueChangeable: exception thrown while "
                               "retrieving the property value.",
                               false);
    }

    return false;
}

void XPropertySet::fillPropsToTest(const uno::Reference<beans::XPropertySetInfo>& xPropInfo)
{
    if (maPropsToTest.initialized)
        return;

    uno::Sequence<beans::Property> aProps = xPropInfo->getProperties();

    // some properties should not be changed in a unspecific way.
    // TODO: Maybe we should mark these properties read-only, instead of
    // giving them a special treatment here?
    std::set<OUString> aSkip;
    aSkip.insert("PrinterName");
    aSkip.insert("CharRelief");
    aSkip.insert("IsLayerMode");

    for (sal_Int32 i = 0; i < aProps.getLength(); ++i)
    {
        beans::Property aProp = aProps[i];
        if (aSkip.count(aProp.Name) > 0)
            continue;

        if ((aProp.Attributes & beans::PropertyAttribute::READONLY) != 0)
        {
            maPropsToTest.readonly.push_back(aProp.Name);
            continue;
        }

        if ((aProp.Attributes & beans::PropertyAttribute::MAYBEVOID) != 0)
            continue;

        bool bBound = (aProp.Attributes & beans::PropertyAttribute::BOUND) != 0;
        bool bConstrained = (aProp.Attributes & beans::PropertyAttribute::CONSTRAINED) != 0;
        bool bCanChange = isPropertyValueChangeable(aProp.Name);

        if (bBound && bCanChange)
            maPropsToTest.bound.push_back(aProp.Name);

        if (bConstrained && bCanChange)
            maPropsToTest.constrained.push_back(aProp.Name);

        if (bCanChange)
            maPropsToTest.normal.push_back(aProp.Name);
    }

    maPropsToTest.initialized = true;
}

bool XPropertySet::getSinglePropertyValue(const uno::Reference<beans::XPropertySet>& xPropSet,
                                          const OUString& rName)
{
    try
    {
        xPropSet->getPropertyValue(rName);
        return true;
    }
    catch (const uno::Exception&)
    {
    }
    return false;
}

bool XPropertySet::isPropertyIgnored(const OUString& rName)
{
    return m_IgnoreValue.count(rName) > 0;
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
