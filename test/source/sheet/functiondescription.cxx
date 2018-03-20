/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/functiondescription.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/FunctionArgument.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void FunctionDescription::testFunctionDescriptionProperties()
{
    uno::Sequence<beans::PropertyValue> aFunctionDescription(init());

    // Only test the the get/read operation of the values, because set/write operation doesn't
    // make any sense. It doesn't trigger any changes.
    // See discussion: nabble.documentfoundation.org/Testing-UNO-API-service-properties-td4236286.html.
    for (auto& value : aFunctionDescription)
    {
        if (value.Name == "Id" || value.Name == "Category")
        {
            sal_Int32 nValueGet = 0;
            OString msg = "Unable to get PropertyValue: "
                          + OUStringToOString(value.Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_ASSERT_MESSAGE(msg.getStr(), value.Value >>= nValueGet);
        }
        else if (value.Name == "Name" || value.Name == "Description")
        {
            OUString sValueGet;
            OString msg = "Unable to get PropertyValue: "
                          + OUStringToOString(value.Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_ASSERT_MESSAGE(msg.getStr(), value.Value >>= sValueGet);
        }
        else if (value.Name == "Arguments")
        {
            uno::Sequence<sheet::FunctionArgument> sArgumentsGet;
            OString msg = "Unable to get PropertyValue: "
                          + OUStringToOString(value.Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_ASSERT_MESSAGE(msg.getStr(), value.Value >>= sArgumentsGet);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong number in set of values for Arguments",
                                         sal_Int32(3), sArgumentsGet.getLength());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value for first description", OUString("Test"),
                                         sArgumentsGet[0].Name);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value for second description",
                                         OUString("Then value"), sArgumentsGet[1].Name);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong value for third description",
                                         OUString("Otherwise value"), sArgumentsGet[2].Name);
        }
        else
        {
            OString aMsg = "Unsupported PropertyValue: "
                           + OUStringToOString(value.Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_FAIL(aMsg.getStr());
        }
    }
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
