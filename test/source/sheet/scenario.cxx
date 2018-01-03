/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <array>
#include <test/sheet/scenario.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void Scenario::testScenarioProperties()
{
    uno::Reference<beans::XPropertySet> xScenario(getScenarioSpreadsheet(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "IsActive";
    bool aIsActive = false;
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aIsActive);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value IsActive", aIsActive);

    aNewValue <<= false;
    xScenario->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aIsActive);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value IsActive", !aIsActive);

    propName = "BorderColor";
    sal_Int32 aBorderColor = 0;
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aBorderColor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value BorderColor", sal_Int32(12632256),
                                 aBorderColor);

    aNewValue <<= sal_Int32(42);
    xScenario->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aBorderColor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set property value BorderColor", sal_Int32(42),
                                 aBorderColor);

    propName = "Protected";
    bool aProtected = false;
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aProtected);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value Protected", aProtected);

    aNewValue <<= false;
    xScenario->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aProtected);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value Protected", !aProtected);

    propName = "ShowBorder";
    bool aShowBorder = false;
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aShowBorder);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value ShowBorder", aShowBorder);

    aNewValue <<= false;
    xScenario->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aShowBorder);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value ShowBorder", !aShowBorder);

    propName = "PrintBorder";
    bool aPrintBorder = false;
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aPrintBorder);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value PrintBorder", aPrintBorder);

    aNewValue <<= false;
    xScenario->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aPrintBorder);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value PrintBorder", !aPrintBorder);

    propName = "CopyBack";
    bool aCopyBack = false;
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aCopyBack);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value CopyBack", aCopyBack);

    aNewValue <<= false;
    xScenario->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aCopyBack);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value CopyBack", !aCopyBack);

    propName = "CopyStyles";
    bool aCopyStyles = true;
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aCopyStyles);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value CopyStyles", !aCopyStyles);

    aNewValue <<= true;
    xScenario->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aCopyStyles);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value CopyStyles", aCopyStyles);

    propName = "CopyFormulas";
    bool aCopyFormulas = false;
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aCopyFormulas);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value CopyFormulas", aCopyFormulas);

    aNewValue <<= false;
    xScenario->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xScenario->getPropertyValue(propName) >>= aCopyFormulas);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value CopyFormulas", !aCopyFormulas);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
