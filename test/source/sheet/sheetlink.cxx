/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/sheetlink.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void SheetLink::testSheetLinkProperties()
{
    uno::Reference<beans::XPropertySet> xSheetLink(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "Url";
    OUString aUrlGet;
    OUString aUrlSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Url",
                           xSheetLink->getPropertyValue(propName) >>= aUrlGet);

    aNewValue <<= OUString("file:///tmp/ScSheetLinkObj.ods");
    xSheetLink->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetLink->getPropertyValue(propName) >>= aUrlSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue Url",
                                 OUString("file:///tmp/ScSheetLinkObj.ods"), aUrlSet);

    propName = "Filter";
    OUString aFilterGet;
    OUString aFilterSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Filter",
                           xSheetLink->getPropertyValue(propName) >>= aFilterGet);

    aNewValue <<= OUString("Text - txt - csv (StarCalc)");
    xSheetLink->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetLink->getPropertyValue(propName) >>= aFilterSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue Filter",
                                 OUString("Text - txt - csv (StarCalc)"), aFilterSet);

    propName = "FilterOptions";
    OUString aFilterOptionsGet;
    OUString aFilterOptionsSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue FilterOptions",
                           xSheetLink->getPropertyValue(propName) >>= aFilterOptionsGet);

    aNewValue <<= OUString("NewValue");
    xSheetLink->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetLink->getPropertyValue(propName) >>= aFilterOptionsSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue FilterOptions", OUString("NewValue"),
                                 aFilterOptionsSet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
