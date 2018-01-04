/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/databaseimportdescriptor.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/util/XImportable.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void DatabaseImportDescriptor::testDatabaseImportDescriptorProperties()
{
    uno::Reference<beans::XPropertySet> xDatabaseImportDescriptor(init(), UNO_QUERY_THROW);
    uno::Reference<util::XImportable> xImportable(getXImportable(), UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aPropValues = xImportable->createImportDescriptor(true);

    for (auto i = 0; i < aPropValues.getLength(); i++)
    {
        uno::Any aOldValue;
        uno::Any aNewValue;
        if (aPropValues[i].Name == "DatabaseName" || aPropValues[i].Name == "SourceObject"
            || aPropValues[i].Name == "ConnectionResource")
        {
            OUString aValue;
            aOldValue = aPropValues[i].Value;
            aOldValue >>= aValue;
            OString aMsgGet = "Unable to get PropertyValue "
                              + OUStringToOString(aPropValues[i].Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsgGet.getStr(), OUString(""), aValue);

            aNewValue <<= OUString("New");
            aPropValues[i].Value = aNewValue;

            aOldValue = aPropValues[i].Value;
            aOldValue >>= aValue;
            OString aMsgSet = "Unable to set PropertyValue "
                              + OUStringToOString(aPropValues[i].Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsgSet.getStr(), OUString("New"), aValue);
        }
        else if (aPropValues[i].Name == "IsNative")
        {
            bool aValue = true;
            aOldValue = aPropValues[i].Value;
            aOldValue >>= aValue;
            OString aMsgGet = "Unable to get PropertyValue "
                              + OUStringToOString(aPropValues[i].Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_ASSERT_MESSAGE(aMsgGet.getStr(), !aValue);

            aNewValue <<= true;
            aPropValues[i].Value = aNewValue;

            aOldValue = aPropValues[i].Value;
            aOldValue >>= aValue;
            OString aMsgSet = "Unable to set PropertyValue "
                              + OUStringToOString(aPropValues[i].Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_ASSERT_MESSAGE(aMsgSet.getStr(), aValue);
        }
        else if (aPropValues[i].Name == "SourceType")
        {
            sheet::DataImportMode aValue;
            aOldValue = aPropValues[i].Value;
            aOldValue >>= aValue;
            OString aMsgGet = "Unable to get PropertyValue "
                              + OUStringToOString(aPropValues[i].Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsgGet.getStr(), sheet::DataImportMode_NONE, aValue);

            aNewValue <<= sheet::DataImportMode_SQL;
            aPropValues[i].Value = aNewValue;

            aOldValue = aPropValues[i].Value;
            aOldValue >>= aValue;
            OString aMsgSet = "Unable to set PropertyValue "
                              + OUStringToOString(aPropValues[i].Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsgSet.getStr(), sheet::DataImportMode_SQL, aValue);
        }
        else
        {
            OString aMsg = "Unsupported PropertyValue "
                           + OUStringToOString(aPropValues[i].Name, RTL_TEXTENCODING_UTF8);
            CPPUNIT_FAIL(aMsg.getStr());
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
