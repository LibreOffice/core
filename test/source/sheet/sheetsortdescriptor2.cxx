/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//#include <test/cppunitasserthelper.hxx>
#include <test/sheet/sheetsortdescriptor2.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/XSortable.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/TableSortField.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

#include <vector>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void SheetSortDescriptor2::testSheetSortDescriptor2Properties()
{
    uno::Reference<util::XSortable> xSortable(init(), UNO_QUERY_THROW);
    const uno::Sequence<beans::PropertyValue> values = xSortable->createSortDescriptor();

    std::vector<OUString> names;
    // Only test the get/read operation of the values, because set/write operation doesn't
    // make any sense. It doesn't trigger any changes.
    // See discussion: nabble.documentfoundation.org/Testing-UNO-API-service-properties-td4236286.html.
    for (const auto& value : values)
    {
        if (value.Name == "BindFormatsToContent")
        {
            names.push_back(value.Name);
            bool bValue = false;
            CPPUNIT_ASSERT(value.Value >>= bValue);
        }
        else if (value.Name == "IsUserListEnabled")
        {
            names.push_back(value.Name);
            bool bValue = false;
            CPPUNIT_ASSERT(value.Value >>= bValue);
        }
        else if (value.Name == "UserListIndex")
        {
            names.push_back(value.Name);
            sal_Int32 nValue = 0;
            CPPUNIT_ASSERT(value.Value >>= nValue);
        }
        else if (value.Name == "CopyOutputData")
        {
            names.push_back(value.Name);
            bool bValue = false;
            CPPUNIT_ASSERT(value.Value >>= bValue);
        }
        else if (value.Name == "OutputPosition")
        {
            names.push_back(value.Name);
            table::CellAddress cellAddr;
            CPPUNIT_ASSERT(value.Value >>= cellAddr);
        }
        else if (value.Name == "SortFields")
        {
            names.push_back(value.Name);
            uno::Sequence<table::TableSortField> sSortFields;
            CPPUNIT_ASSERT(value.Value >>= sSortFields);
        }
        else if (value.Name == "MaxFieldCount")
        {
            names.push_back(value.Name);
            sal_Int32 nValue = 0;
            CPPUNIT_ASSERT(value.Value >>= nValue);
        }
        else if (value.Name == "IsSortColumns")
        {
            names.push_back(value.Name);
            bool bValue = false;
            CPPUNIT_ASSERT(value.Value >>= bValue);
        }
        else if (value.Name == "ContainsHeader")
        {
            names.push_back(value.Name);
            bool bValue = false;
            CPPUNIT_ASSERT(value.Value >>= bValue);
        }
        else
        {
            OString sMsg = "Unsupported PropertyValue: "
                           + OUStringToOString(value.Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_FAIL(sMsg.getStr());
        }
    }

    CPPUNIT_ASSERT_MESSAGE("Property BindFormatsToContent not found",
                           std::count(std::begin(names), std::end(names), "BindFormatsToContent"));
    CPPUNIT_ASSERT_MESSAGE("Property IsUserListEnabled not found",
                           std::count(std::begin(names), std::end(names), "IsUserListEnabled"));
    CPPUNIT_ASSERT_MESSAGE("Property UserListIndex not found",
                           std::count(std::begin(names), std::end(names), "UserListIndex"));
    CPPUNIT_ASSERT_MESSAGE("Property CopyOutputData not found",
                           std::count(std::begin(names), std::end(names), "CopyOutputData"));
    CPPUNIT_ASSERT_MESSAGE("Property OutputPosition not found",
                           std::count(std::begin(names), std::end(names), "OutputPosition"));
    CPPUNIT_ASSERT_MESSAGE("Property SortFields not found",
                           std::count(std::begin(names), std::end(names), "SortFields"));
    CPPUNIT_ASSERT_MESSAGE("Property MaxFieldCount not found",
                           std::count(std::begin(names), std::end(names), "MaxFieldCount"));
    CPPUNIT_ASSERT_MESSAGE("Property IsSortColumns not found",
                           std::count(std::begin(names), std::end(names), "IsSortColumns"));
    CPPUNIT_ASSERT_MESSAGE("Property ContainsHeader not found",
                           std::count(std::begin(names), std::end(names), "ContainsHeader"));
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
