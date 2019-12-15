/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/cppunitasserthelper.hxx>
#include <test/sheet/sheetfilterdescriptor.hxx>

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
void SheetFilterDescriptor::testSheetFilterDescriptorProperties()
{
    uno::Reference<beans::XPropertySet> xSheetFilterDescriptor(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "IsCaseSensitive";
    bool bIsCaseSensitiveGet = false;
    bool bIsCaseSensitiveSet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsCaseSensitive",
                           xSheetFilterDescriptor->getPropertyValue(propName)
                           >>= bIsCaseSensitiveGet);

    aNewValue <<= !bIsCaseSensitiveGet;
    xSheetFilterDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetFilterDescriptor->getPropertyValue(propName) >>= bIsCaseSensitiveSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue IsCaseSensitive",
                                 !bIsCaseSensitiveGet, bIsCaseSensitiveSet);

    propName = "SkipDuplicates";
    bool bSkipDuplicatesGet = false;
    bool bSkipDuplicatesSet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue SkipDuplicates",
                           xSheetFilterDescriptor->getPropertyValue(propName)
                           >>= bSkipDuplicatesGet);

    aNewValue <<= !bSkipDuplicatesGet;
    xSheetFilterDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetFilterDescriptor->getPropertyValue(propName) >>= bSkipDuplicatesSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue SkipDuplicates", !bSkipDuplicatesGet,
                                 bSkipDuplicatesSet);

    propName = "UseRegularExpressions";
    bool bUseRegularExpressionsGet = false;
    bool bUseRegularExpressionsSet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue UseRegularExpressions",
                           xSheetFilterDescriptor->getPropertyValue(propName)
                           >>= bUseRegularExpressionsGet);

    aNewValue <<= !bUseRegularExpressionsGet;
    xSheetFilterDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetFilterDescriptor->getPropertyValue(propName)
                   >>= bUseRegularExpressionsSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue UseRegularExpressions",
                                 !bUseRegularExpressionsGet, bUseRegularExpressionsSet);

    propName = "SaveOutputPosition";
    bool bSaveOutputPositionGet = false;
    bool bSaveOutputPositionSet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue SaveOutputPosition",
                           xSheetFilterDescriptor->getPropertyValue(propName)
                           >>= bSaveOutputPositionGet);

    aNewValue <<= !bSaveOutputPositionGet;
    xSheetFilterDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetFilterDescriptor->getPropertyValue(propName) >>= bSaveOutputPositionSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue SaveOutputPosition",
                                 !bSaveOutputPositionGet, bSaveOutputPositionSet);

    propName = "Orientation";
    table::TableOrientation aOrientationGet;
    table::TableOrientation aOrientationSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Orientation",
                           xSheetFilterDescriptor->getPropertyValue(propName) >>= aOrientationGet);

    aNewValue <<= table::TableOrientation_COLUMNS;
    xSheetFilterDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetFilterDescriptor->getPropertyValue(propName) >>= aOrientationSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue Orientation",
                                 table::TableOrientation_COLUMNS, aOrientationSet);

    propName = "ContainsHeader";
    bool bContainsHeaderGet = false;
    bool bContainsHeaderSet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ContainsHeader",
                           xSheetFilterDescriptor->getPropertyValue(propName)
                           >>= bContainsHeaderGet);

    aNewValue <<= !bContainsHeaderGet;
    xSheetFilterDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetFilterDescriptor->getPropertyValue(propName) >>= bContainsHeaderSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue ContainsHeader", !bContainsHeaderGet,
                                 bContainsHeaderSet);

    propName = "CopyOutputData";
    bool bCopyOutputDataGet = false;
    bool bCopyOutputDataSet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue CopyOutputData",
                           xSheetFilterDescriptor->getPropertyValue(propName)
                           >>= bCopyOutputDataGet);

    aNewValue <<= !bCopyOutputDataGet;
    xSheetFilterDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetFilterDescriptor->getPropertyValue(propName) >>= bCopyOutputDataSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue CopyOutputData", !bCopyOutputDataGet,
                                 bCopyOutputDataSet);

    propName = "OutputPosition";
    table::CellAddress aCellAddressGet;
    table::CellAddress aCellAddressSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue OutputPosition",
                           xSheetFilterDescriptor->getPropertyValue(propName) >>= aCellAddressGet);

    aCellAddressGet = table::CellAddress(0, 42, 42);
    aNewValue <<= aCellAddressGet;
    xSheetFilterDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetFilterDescriptor->getPropertyValue(propName) >>= aCellAddressSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue OutputPosition", aCellAddressGet,
                                 aCellAddressSet);

    propName = "MaxFieldCount";
    sal_Int32 nMaxFieldCountGet = 0;
    sal_Int32 nMaxFieldCountSet = 0;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue MaxFieldCount",
                           xSheetFilterDescriptor->getPropertyValue(propName)
                           >>= nMaxFieldCountGet);

    aNewValue <<= nMaxFieldCountGet + 42;
    xSheetFilterDescriptor->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetFilterDescriptor->getPropertyValue(propName) >>= nMaxFieldCountSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Able to set PropertyValue MaxFieldCount", nMaxFieldCountGet,
                                 nMaxFieldCountSet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
