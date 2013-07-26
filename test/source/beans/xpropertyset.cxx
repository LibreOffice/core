/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test/beans/xpropertyset.hxx"
#include "cppunit/extensions/HelperMacros.h"

#include <com/sun/star/uno/Type.h>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <set>

using namespace css;
using namespace css::uno;

namespace apitest {

XPropertySet::~XPropertySet() {}

XPropertySet::PropsToTest::PropsToTest() : initialized(false) {}

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

void XPropertySet::testAddPropertyChangeListener()
{
    // TODO: implement this.
}

void XPropertySet::testAddVetoableChangeListener()
{
    // TODO: implement this.
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

void XPropertySet::testRemovePropertyChangeListener()
{
    // TODO: implement this.
}

void XPropertySet::testRemoveVetoableChangeListener()
{
    // TODO: implement this.
}

bool XPropertySet::isPropertyValueChangeable(const OUString& rName)
{
    uno::Reference<beans::XPropertySet> xPropSet(init(), UNO_QUERY_THROW);
    try
    {
        uno::Any any = xPropSet->getPropertyValue(rName);
        uno::Type type = any.getValueType();
        if (type == getCppuType<sal_Bool>())
        {
            // boolean type
            sal_Bool bOld = any.get<sal_Bool>();
            xPropSet->setPropertyValue(rName, makeAny(!bOld));
        }
        else if (type == getCppuType<sal_Int8>())
        {
            // 8-bit integer
            sal_Int8 nOld = any.get<sal_Int8>();
            sal_Int8 nNew = nOld + 1;
            xPropSet->setPropertyValue(rName, makeAny(nNew));
        }
        else if (type == getCppuType<sal_Int16>())
        {
            // 16-bit integer
            sal_Int16 nOld = any.get<sal_Int16>();
            sal_Int16 nNew = nOld + 2;
            xPropSet->setPropertyValue(rName, makeAny(nNew));
        }
        else if (type == getCppuType<sal_Int32>())
        {
            // 32-bit integer
            sal_Int32 nOld = any.get<sal_Int32>();
            sal_Int32 nNew = nOld + 3;
            xPropSet->setPropertyValue(rName, makeAny(nNew));
        }
        else if (type == getCppuType<sal_Int64>())
        {
            // 64-bit integer
            sal_Int64 nOld = any.get<sal_Int64>();
            sal_Int64 nNew = nOld + 4;
            xPropSet->setPropertyValue(rName, makeAny(nNew));
        }
        else if (type == getCppuType<float>())
        {
            // single precision
            float fOld = any.get<float>();
            float fNew = fOld + 1.2;
            xPropSet->setPropertyValue(rName, makeAny(fNew));
        }
        else if (type == getCppuType<double>())
        {
            // double precision
            double fOld = any.get<double>();
            double fNew = fOld + 1.3;
            xPropSet->setPropertyValue(rName, makeAny(fNew));
        }
        else if (type == getCppuType<OUString>())
        {
            // string type
            OUString aOld = any.get<OUString>();
            OUString aNew = aOld + OUString("foo");
            xPropSet->setPropertyValue(rName, makeAny(aNew));
        }
        else if (type == getCppuType<util::DateTime>())
        {
            // date time type
            util::DateTime aDT = any.get<util::DateTime>();
            aDT.Year += 1;
            xPropSet->setPropertyValue(rName, makeAny(aDT));
        }
        else
        {
            CPPUNIT_ASSERT_MESSAGE("XPropertySet::isChangeable: unknown type in Any tested.", false);
        }

        uno::Any anyTest = xPropSet->getPropertyValue(rName);
        return any != anyTest;
    }
    catch (const uno::Exception&)
    {
        CPPUNIT_ASSERT_MESSAGE("XPropertySet::isChangeable: exception thrown while retrieving the property value.", false);
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

        bool bBound       = (aProp.Attributes & beans::PropertyAttribute::BOUND) != 0;
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

bool XPropertySet::getSinglePropertyValue(
    const uno::Reference<beans::XPropertySet>& xPropSet, const OUString& rName)
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
