/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/cppunitasserthelper.hxx>
#include <test/text/numberingrules.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void NumberingRules::testNumberingRulesProperties()
{
    Reference<beans::XPropertySet> xNumberingRules(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "IsAbsoluteMargins";
    bool bIsAbsoluteMarginsGet = false;
    bool bIsAbsoluteMarginsSet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsAbsoluteMargins",
                           xNumberingRules->getPropertyValue(propName) >>= bIsAbsoluteMarginsGet);

    aNewValue <<= !bIsAbsoluteMarginsGet;
    xNumberingRules->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xNumberingRules->getPropertyValue(propName) >>= bIsAbsoluteMarginsSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue IsAbsoluteMargins",
                                 !bIsAbsoluteMarginsGet, bIsAbsoluteMarginsSet);

    propName = "IsAutomatic";
    bool bIsAutomaticGet = false;
    bool bIsAutomaticSet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsAutomatic",
                           xNumberingRules->getPropertyValue(propName) >>= bIsAutomaticGet);

    bIsAutomaticGet = false;
    aNewValue <<= !bIsAutomaticGet;
    xNumberingRules->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xNumberingRules->getPropertyValue(propName) >>= bIsAutomaticSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue IsAutomatic", !bIsAutomaticGet,
                                 bIsAutomaticSet);

    propName = "IsContinuousNumbering";
    bool bIsContinuousNumberingGet = false;
    bool bIsContinuousNumberingSet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsContinuousNumbering",
                           xNumberingRules->getPropertyValue(propName)
                           >>= bIsContinuousNumberingGet);

    aNewValue <<= !bIsContinuousNumberingGet;
    xNumberingRules->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xNumberingRules->getPropertyValue(propName) >>= bIsContinuousNumberingSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue IsContinuousNumbering",
                                 !bIsContinuousNumberingGet, bIsContinuousNumberingSet);

    propName = "Name";
    OUString aNameGet = u""_ustr;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Name",
                           xNumberingRules->getPropertyValue(propName) >>= aNameGet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
