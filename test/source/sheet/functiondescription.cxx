/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>

#include <test/sheet/functiondescription.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/FunctionArgument.hpp>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void FunctionDescription::testFunctionDescriptionProperties()
{
    const uno::Sequence<beans::PropertyValue> aFunctionDescription(init());

    std::vector<OUString> names;
    // Only test the get/read operation of the values, because set/write operation doesn't
    // make any sense. It doesn't trigger any changes.
    // See discussion: nabble.documentfoundation.org/Testing-UNO-API-service-properties-td4236286.html.
    for (const auto& value : aFunctionDescription)
    {
        if (value.Name == "Id")
        {
            names.push_back(value.Name);
            sal_Int32 nValue = 0;
            CPPUNIT_ASSERT(value.Value >>= nValue);
        }
        else if (value.Name == "Category")
        {
            names.push_back(value.Name);
            sal_Int32 nValue = 0;
            CPPUNIT_ASSERT(value.Value >>= nValue);
        }
        else if (value.Name == "Name")
        {
            names.push_back(value.Name);
            OUString sValue;
            CPPUNIT_ASSERT(value.Value >>= sValue);
        }
        else if (value.Name == "Description")
        {
            names.push_back(value.Name);
            OUString sValue;
            CPPUNIT_ASSERT(value.Value >>= sValue);
        }
        else if (value.Name == "Arguments")
        {
            names.push_back(value.Name);
            uno::Sequence<sheet::FunctionArgument> sArguments;
            CPPUNIT_ASSERT(value.Value >>= sArguments);
        }
        else
        {
            OString aMsg = "Unsupported PropertyValue: "
                           + OUStringToOString(value.Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_FAIL(aMsg.getStr());
        }
    }

    CPPUNIT_ASSERT_MESSAGE("Property Id not found",
                           std::count(std::begin(names), std::end(names), "Id"));
    CPPUNIT_ASSERT_MESSAGE("Property Category not found",
                           std::count(std::begin(names), std::end(names), "Category"));
    CPPUNIT_ASSERT_MESSAGE("Property Name not found",
                           std::count(std::begin(names), std::end(names), "Name"));
    CPPUNIT_ASSERT_MESSAGE("Property Description not found",
                           std::count(std::begin(names), std::end(names), "Description"));
    CPPUNIT_ASSERT_MESSAGE("Property Arguments not found",
                           std::count(std::begin(names), std::end(names), "Arguments"));
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
