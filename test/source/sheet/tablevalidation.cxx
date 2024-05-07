/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/tablevalidation.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void TableValidation::testTableValidationProperties()
{
    uno::Reference<beans::XPropertySet> xTableValidation(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "Type";
    uno::Any aValue = xTableValidation->getPropertyValue(propName);
    sheet::ValidationType aValidationType;
    aValue >>= aValidationType;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value Type", sheet::ValidationType_ANY,
                                 aValidationType);

    aNewValue <<= sheet::ValidationType_WHOLE;
    xTableValidation->setPropertyValue(propName, aNewValue);
    aValue = xTableValidation->getPropertyValue(propName);
    aValue >>= aValidationType;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set property value Type", sheet::ValidationType_WHOLE,
                                 aValidationType);

    propName = "ShowInputMessage";
    bool aShowInputMessage = true;
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aShowInputMessage);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value ShowInputMessage", !aShowInputMessage);

    aNewValue <<= true;
    xTableValidation->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aShowInputMessage);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value ShowInputMessage", aShowInputMessage);

    // only possible if ShowInputMessage is true
    propName = "InputMessage";
    OUString aInputMessage;
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aInputMessage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value InputMessage", OUString(""),
                                 aInputMessage);

    aNewValue <<= OUString("UnitTest InputMessage");
    xTableValidation->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aInputMessage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value InputMessage",
                                 OUString("UnitTest InputMessage"), aInputMessage);

    // only possible if ShowInputMessage is true
    propName = "InputTitle";
    OUString aInputTitle;
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aInputTitle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value InputTitle", OUString(""),
                                 aInputTitle);

    aNewValue <<= OUString("UnitTest InputTitle");
    xTableValidation->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aInputTitle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value InputTitle",
                                 OUString("UnitTest InputTitle"), aInputTitle);

    propName = "ShowErrorMessage";
    bool aShowErrorMessage = true;
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aShowErrorMessage);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value ShowErrorMessage", !aShowErrorMessage);

    aNewValue <<= true;
    xTableValidation->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aShowErrorMessage);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value ShowErrorMessage", aShowErrorMessage);

    // only possible if ShowErrorMessage is true
    propName = "ErrorMessage";
    OUString aErrorMessage;
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aErrorMessage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value ErrorMessage", OUString(""),
                                 aErrorMessage);

    aNewValue <<= OUString("UnitTest ErrorMessage");
    xTableValidation->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aErrorMessage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value ErrorMessage",
                                 OUString("UnitTest ErrorMessage"), aErrorMessage);

    // only possible if ShowErrorMessage is true
    propName = "ErrorTitle";
    OUString aErrorTitle;
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aErrorTitle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value ErrorTitle", OUString(""),
                                 aErrorTitle);

    aNewValue <<= OUString("UnitTest ErrorTitle");
    xTableValidation->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aErrorTitle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value ErrorTitle",
                                 OUString("UnitTest ErrorTitle"), aErrorTitle);

    propName = "IgnoreBlankCells";
    bool aIgnoreBlankCells = false;
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aIgnoreBlankCells);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value IgnoreBlankCells", aIgnoreBlankCells);

    aNewValue <<= false;
    xTableValidation->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aIgnoreBlankCells);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value IgnoreBlankCells", !aIgnoreBlankCells);

    propName = "IsCaseSensitive";
    bool bCaseSensitive = false;
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= bCaseSensitive);
    CPPUNIT_ASSERT_MESSAGE("Unable to get property value IsCaseSensitive", !bCaseSensitive);

    aNewValue <<= true;
    xTableValidation->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= bCaseSensitive);
    CPPUNIT_ASSERT_MESSAGE("Unable to set property value IsCaseSensitive", bCaseSensitive);

    propName = "ErrorAlertStyle";
    aValue = xTableValidation->getPropertyValue(propName);
    sheet::ValidationAlertStyle aValidationAlertStyle;
    aValue >>= aValidationAlertStyle;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value ValidationAlertStyle",
                                 sheet::ValidationAlertStyle_STOP, aValidationAlertStyle);

    aNewValue <<= sheet::ValidationAlertStyle_WARNING;
    xTableValidation->setPropertyValue(propName, aNewValue);
    aValue = xTableValidation->getPropertyValue(propName);
    aValue >>= aValidationAlertStyle;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set property value ValidationAlertStyle",
                                 sheet::ValidationAlertStyle_WARNING, aValidationAlertStyle);

    propName = "ShowList";
    sal_Int16 aShowList = 0;
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aShowList);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get property value ShowList", sal_Int16(1), aShowList);

    aNewValue <<= sal_Int16(42);
    xTableValidation->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xTableValidation->getPropertyValue(propName) >>= aShowList);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set property value ShowList", sal_Int16(42), aShowList);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
