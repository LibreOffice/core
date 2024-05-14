/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/cppunitasserthelper.hxx>
#include <test/text/baseindexmark.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void BaseIndexMark::testBaseIndexMarkProperties()
{
    Reference<beans::XPropertySet> xBaseIndexMark(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "AlternativeText";
    OUString aAlternativeTextGet = u""_ustr;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue AlternativeText",
                           xBaseIndexMark->getPropertyValue(propName) >>= aAlternativeTextGet);

    OUString aAlternativeTextSet = u"testProperty"_ustr;
    aNewValue <<= aAlternativeTextSet;
    xBaseIndexMark->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xBaseIndexMark->getPropertyValue(propName) >>= aAlternativeTextGet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue AlternativeText", aAlternativeTextGet,
                                 aAlternativeTextSet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
