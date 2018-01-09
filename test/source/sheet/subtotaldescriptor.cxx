/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/subtotaldescriptor.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void SubTotalDescriptor::testSubTotalDescriptorProperties()
{
    uno::Reference<beans::XPropertySet> xSubTotalDescriptor(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "InsertPageBreaks";
    bool aInsertPageBreaks = true;
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aInsertPageBreaks);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue InsertPageBreaks", !aInsertPageBreaks);

    aNewValue <<= true;
    xSubTotalDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aInsertPageBreaks);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue InsertPageBreaks", aInsertPageBreaks);

    propName = "IsCaseSensitive";
    bool aIsCaseSensitive = true;
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aIsCaseSensitive);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsCaseSensitive", !aIsCaseSensitive);

    aNewValue <<= true;
    xSubTotalDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aIsCaseSensitive);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue IsCaseSensitive", aIsCaseSensitive);

    propName = "EnableUserSortList";
    bool aEnableUserSortList = true;
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aEnableUserSortList);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue EnableUserSortList", !aEnableUserSortList);

    aNewValue <<= true;
    xSubTotalDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aEnableUserSortList);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue EnableUserSortList", aEnableUserSortList);

    propName = "UserSortListIndex";
    sal_Int32 aUserSortListIndex = 42;
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aUserSortListIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue UserSortListIndex", sal_Int32(0),
                                 aUserSortListIndex);

    aNewValue <<= sal_Int32(42);
    xSubTotalDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aUserSortListIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue UserSortListIndex", sal_Int32(42),
                                 aUserSortListIndex);

    propName = "BindFormatsToContent";
    bool aBindFormatsToContent = true;
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aBindFormatsToContent);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue BindFormatsToContent",
                           !aBindFormatsToContent);

    aNewValue <<= true;
    xSubTotalDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aBindFormatsToContent);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue BindFormatsToContent",
                           aBindFormatsToContent);

    propName = "EnableSort";
    bool aEnableSort = false;
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aEnableSort);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue EnableSort", aEnableSort);

    aNewValue <<= false;
    xSubTotalDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aEnableSort);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue EnableSort", !aEnableSort);

    propName = "SortAscending";
    bool aSortAscending = false;
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aSortAscending);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue SortAscending", aSortAscending);

    aNewValue <<= false;
    xSubTotalDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aSortAscending);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue SortAscending", !aSortAscending);

    propName = "MaxFieldCount";
    sal_Int32 aMaxFieldCount = 42;
    CPPUNIT_ASSERT(xSubTotalDescriptor->getPropertyValue(propName) >>= aMaxFieldCount);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue MaxFieldCount", sal_Int32(3),
                                 aMaxFieldCount);

    aNewValue <<= sal_Int32(42);
    CPPUNIT_ASSERT_THROW_MESSAGE("Able to change PropertyValue MaxFieldCount",
                                 xSubTotalDescriptor->setPropertyValue(propName, aNewValue),
                                 lang::IllegalArgumentException);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
