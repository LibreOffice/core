/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/cppunitasserthelper.hxx>
#include <test/text/documentindexmark.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void DocumentIndexMark::testDocumentIndexMarkProperties()
{
    Reference<beans::XPropertySet> xDocumentIndexMark(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "PrimaryKey";
    OUString aPrimaryKeyGet = u""_ustr;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue PrimaryKey",
                           xDocumentIndexMark->getPropertyValue(propName) >>= aPrimaryKeyGet);

    OUString aPrimaryKeySet = u"testPropertyPK"_ustr;
    aNewValue <<= aPrimaryKeySet;
    xDocumentIndexMark->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentIndexMark->getPropertyValue(propName) >>= aPrimaryKeyGet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue PrimaryKey", aPrimaryKeyGet,
                                 aPrimaryKeySet);

    propName = "SecondaryKey";
    OUString aSecondaryKeyGet = u""_ustr;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue SecondaryKey",
                           xDocumentIndexMark->getPropertyValue(propName) >>= aSecondaryKeyGet);

    OUString aSecondaryKeySet = u"testPropertySK"_ustr;
    aNewValue <<= aSecondaryKeySet;
    xDocumentIndexMark->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentIndexMark->getPropertyValue(propName) >>= aSecondaryKeyGet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue SecondaryKey", aSecondaryKeyGet,
                                 aSecondaryKeySet);

    propName = "IsMainEntry";
    bool bIsMainEntryGet = false;
    bool bIsMainEntrySet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsMainEntry",
                           xDocumentIndexMark->getPropertyValue(propName) >>= bIsMainEntryGet);

    aNewValue <<= !bIsMainEntryGet;
    xDocumentIndexMark->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentIndexMark->getPropertyValue(propName) >>= bIsMainEntrySet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue IsMainEntry", !bIsMainEntryGet,
                                 bIsMainEntrySet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
