/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/unoapi_test.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/oustringostreaminserter.hxx>

namespace ScDatabaseRangeObj {

class ScXDatabaseRange : public UnoApiTest
{
    uno::Reference< sheet::XDatabaseRange > init(const rtl::OUString& rName);

    void testDataArea();
    void testGetSortDescriptor();
    void testGetSubtotalDescriptor();

    CPPUNIT_TEST_SUITE(ScXDatabaseRange);
    CPPUNIT_TEST(testDataArea);
    CPPUNIT_TEST(testGetSortDescriptor);
    CPPUNIT_TEST(testGetSubtotalDescriptor);
    CPPUNIT_TEST_SUITE_END();
};

/**
 * tests setDataArea and getDataArea
 */
void ScXDatabaseRange::testDataArea()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange = init(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataArea")));

    table::CellRangeAddress aCellAddress;
    aCellAddress.Sheet = 0;
    aCellAddress.StartColumn = 1;
    aCellAddress.EndColumn = 4;
    aCellAddress.StartRow = 2;
    aCellAddress.EndRow = 5;
    xDBRange->setDataArea(aCellAddress);
    table::CellRangeAddress aValue;
    aValue = xDBRange->getDataArea();
    CPPUNIT_ASSERT( aCellAddress.Sheet == aValue.Sheet );
    CPPUNIT_ASSERT( aCellAddress.StartRow == aValue.StartRow );
    CPPUNIT_ASSERT( aCellAddress.EndRow == aValue.EndRow );
    CPPUNIT_ASSERT( aCellAddress.StartColumn == aValue.StartColumn );
    CPPUNIT_ASSERT( aCellAddress.EndColumn == aValue.EndColumn );
}

void ScXDatabaseRange::testGetSubtotalDescriptor()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange = init(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SubtotalDescriptor")));
    uno::Reference< sheet::XSubTotalDescriptor> xSubtotalDescr = xDBRange->getSubTotalDescriptor();
    CPPUNIT_ASSERT(xSubtotalDescr.is());
}

void ScXDatabaseRange::testGetSortDescriptor()
{
    uno::Reference< sheet::XDatabaseRange > xDBRange = init(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SortDescriptor")));
    uno::Sequence< beans::PropertyValue > xSortDescr = xDBRange->getSortDescriptor();
    for (sal_Int32 i = 0; i < xSortDescr.getLength(); ++i)
    {
        beans::PropertyValue xProp = xSortDescr[i];
        //std::cout << "Prop " << i << " Name: " << rtl::OUString(xProp.Name) << std::endl;

        if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsSortColumns")))
        {
            sal_Bool bIsSortColumns;
            xProp.Value >>= bIsSortColumns;
            CPPUNIT_ASSERT(bIsSortColumns == false);
        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ContainsHeader")))
        {
            sal_Bool bContainsHeader;
            xProp.Value >>= bContainsHeader;
            CPPUNIT_ASSERT(bContainsHeader == true);
        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MaxFieldCount")))
        {
            sal_Int32 nMaxFieldCount;
            xProp.Value >>= nMaxFieldCount;
            std::cout << "Value: " << nMaxFieldCount << std::endl;

        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SortFields")))
        {

        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BindFormatsToContent")))
        {
            sal_Bool bBindFormatsToContent;
            xProp.Value >>= bBindFormatsToContent;
            CPPUNIT_ASSERT(bBindFormatsToContent == true);
        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CopyOutputData")))
        {
            sal_Bool bCopyOutputData;
            xProp.Value >>= bCopyOutputData;
            CPPUNIT_ASSERT(bCopyOutputData == false);
        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OutputPosition")))
        {

        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsUserListEnabled")))
        {
            sal_Bool bIsUserListEnabled;
            xProp.Value >>= bIsUserListEnabled;
            CPPUNIT_ASSERT(bIsUserListEnabled == false);

        }
        else if (xProp.Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UserListIndex")))
        {
            sal_Int32 nUserListIndex;
            xProp.Value >>= nUserListIndex;
            CPPUNIT_ASSERT(nUserListIndex == 0);
        }
    }
}

uno::Reference< sheet::XDatabaseRange > ScXDatabaseRange::init(const rtl::OUString& rName)
{
    rtl::OUString aFileURL;
    const rtl::OUString aFileBase(RTL_CONSTASCII_USTRINGPARAM("xdatabaserange.ods"));
    createFileURL(aFileBase, aFileURL);
    std::cout << rtl::OUStringToOString(aFileURL, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    static uno::Reference< lang::XComponent > xComponent;
    if( !xComponent.is())
        xComponent = loadFromDesktop(aFileURL);
    uno::Reference< sheet::XSpreadsheetDocument> xDoc (xComponent, UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xPropSet(xDoc,UNO_QUERY_THROW);
    uno::Reference< sheet::XDatabaseRanges > xDBRanges( xPropSet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DatabaseRanges"))), UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xName( xDBRanges, UNO_QUERY_THROW);
    uno::Reference< sheet::XDatabaseRange > xDBRange( xName->getByName(rName), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDBRange.is());
    return xDBRange;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScXDatabaseRange);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
