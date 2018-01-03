/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/tableautoformat.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void TableAutoFormat::testTableAutoFormatProperties()
{
    uno::Reference<beans::XPropertySet> xTableAutoFormat(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "IncludeFont";
    bool aIncludeFont = false;
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeFont);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value IncludeFont", aIncludeFont);

    aNewValue <<= false;
    xTableAutoFormat->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeFont);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value IncludeFont", !aIncludeFont);

    propName = "IncludeJustify";
    bool aIncludeJustify = false;
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeJustify);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value IncludeJustify", aIncludeJustify);

    aNewValue <<= false;
    xTableAutoFormat->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeJustify);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value IncludeJustify", !aIncludeJustify);

    propName = "IncludeBorder";
    bool aIncludeBorder = false;
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeBorder);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value IncludeBorder", aIncludeBorder);

    aNewValue <<= false;
    xTableAutoFormat->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeBorder);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value IncludeBorder", !aIncludeBorder);

    propName = "IncludeBackground";
    bool aIncludeBackground = false;
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeBackground);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value IncludeBackground", aIncludeBackground);

    aNewValue <<= false;
    xTableAutoFormat->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeBackground);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value IncludeBackground", !aIncludeBackground);

    propName = "IncludeNumberFormat";
    bool aIncludeNumberFormat = false;
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeNumberFormat);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value IncludeNumberFormat",
                           aIncludeNumberFormat);

    aNewValue <<= false;
    xTableAutoFormat->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeNumberFormat);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value IncludeNumberFormat",
                           !aIncludeNumberFormat);

    propName = "IncludeWidthAndHeight";
    bool aIncludeWidthAndHeight = false;
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeWidthAndHeight);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value IncludeWidthAndHeight",
                           aIncludeWidthAndHeight);

    aNewValue <<= false;
    xTableAutoFormat->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableAutoFormat->getPropertyValue(propName) >>= aIncludeWidthAndHeight);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value IncludeWidthAndHeight",
                           !aIncludeWidthAndHeight);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
