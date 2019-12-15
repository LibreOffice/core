/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_property_testers.hxx>
#include <test/sheet/spreadsheetdocumentsettings.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void SpreadsheetDocumentSettings::testSpreadsheetDocumentSettingsProperties()
{
    uno::Reference<beans::XPropertySet> xSpreadsheetDocumentSettings(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "IsIterationEnabled";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "IterationCount";
    testLongProperty(xSpreadsheetDocumentSettings, propName);

    propName = "IterationEpsilon";
    testDoubleProperty(xSpreadsheetDocumentSettings, propName);

    propName = "StandardDecimals";
    testShortProperty(xSpreadsheetDocumentSettings, propName);

    propName = "NullDate";
    util::Date aNullDateGet;
    util::Date aNullDateSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: NullDate",
                           xSpreadsheetDocumentSettings->getPropertyValue(propName)
                           >>= aNullDateGet);

    aNewValue <<= util::Date(1, 1, 2000);
    xSpreadsheetDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetDocumentSettings->getPropertyValue(propName) >>= aNullDateSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue: NullDate", sal_Int16(2000),
                                 aNullDateSet.Year);

    propName = "DefaultTabStop";
    testShortProperty(xSpreadsheetDocumentSettings, propName);

    propName = "IgnoreCase";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "CalcAsShown";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "MatchWholeCell";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "SpellOnline";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "LookUpLabels";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "RegularExpressions";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "ForbiddenCharacters";
    uno::Reference<i18n::XForbiddenCharacters> xForbiddenCharactersGet;
    uno::Reference<i18n::XForbiddenCharacters> xForbiddenCharactersSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: ForbiddenCharacters",
                           xSpreadsheetDocumentSettings->getPropertyValue(propName)
                           >>= xForbiddenCharactersGet);

    lang::Locale aForbiddenCharactersLocale;
    aForbiddenCharactersLocale.Language = "ru";
    aForbiddenCharactersLocale.Country = "RU";

    uno::Reference<i18n::XForbiddenCharacters> xForbiddenCharactersNew;
    aNewValue <<= xForbiddenCharactersNew;
    xSpreadsheetDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetDocumentSettings->getPropertyValue(propName)
                   >>= xForbiddenCharactersSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Able to set PropertyValue: ForbiddenCharacters",
        xForbiddenCharactersGet->hasForbiddenCharacters(aForbiddenCharactersLocale),
        xForbiddenCharactersSet->hasForbiddenCharacters(aForbiddenCharactersLocale));

    propName = "HasDrawPages";
    testBooleanReadonlyProperty(xSpreadsheetDocumentSettings, propName);

    propName = "CharLocale";
    lang::Locale aCharLocaleGet;
    lang::Locale aCharLocaleSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: CharLocale",
                           xSpreadsheetDocumentSettings->getPropertyValue(propName)
                           >>= aCharLocaleGet);

    lang::Locale aCharLocaleNew;
    aCharLocaleNew.Language = "en";
    aCharLocaleNew.Country = "CA";
    aNewValue <<= aCharLocaleNew;
    xSpreadsheetDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetDocumentSettings->getPropertyValue(propName) >>= aCharLocaleSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue: CharLocale", aCharLocaleNew.Country,
                                 aCharLocaleSet.Country);

    propName = "CharLocaleAsian";
    lang::Locale aCharLocaleAsianGet;
    lang::Locale aCharLocaleAsianSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: CharLocaleAsian",
                           xSpreadsheetDocumentSettings->getPropertyValue(propName)
                           >>= aCharLocaleAsianGet);

    lang::Locale aCharLocaleAsianNew;
    aCharLocaleAsianNew.Language = "en";
    aCharLocaleAsianNew.Country = "CA";
    aNewValue <<= aCharLocaleAsianNew;
    xSpreadsheetDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetDocumentSettings->getPropertyValue(propName)
                   >>= aCharLocaleAsianSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue: CharLocaleAsian",
                                 aCharLocaleAsianNew.Country, aCharLocaleAsianSet.Country);

    propName = "CharLocaleComplex";
    lang::Locale aCharLocaleComplexGet;
    lang::Locale aCharLocaleComplexSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: CharLocaleComplex",
                           xSpreadsheetDocumentSettings->getPropertyValue(propName)
                           >>= aCharLocaleComplexGet);

    lang::Locale aCharLocaleComplexNew;
    aCharLocaleComplexNew.Language = "en";
    aCharLocaleComplexNew.Country = "CA";
    aNewValue <<= aCharLocaleComplexNew;
    xSpreadsheetDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetDocumentSettings->getPropertyValue(propName)
                   >>= aCharLocaleComplexSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue: CharLocaleComplex",
                                 aCharLocaleComplexNew.Country, aCharLocaleComplexSet.Country);

    propName = "IsLoaded";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "IsUndoEnabled";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "IsAdjustHeightEnabled";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "IsExecuteLinkEnabled";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "ReferenceDevice";
    uno::Reference<awt::XDevice> xDeviceGet;
    uno::Reference<awt::XDevice> xDeviceSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: ReferenceDevice",
                           xSpreadsheetDocumentSettings->getPropertyValue(propName) >>= xDeviceGet);

    uno::Reference<awt::XDevice> xDeviceNew;
    aNewValue <<= xDeviceNew;
    xSpreadsheetDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetDocumentSettings->getPropertyValue(propName) >>= xDeviceSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Able to set PropertyValue: ReferenceDevice",
                                 xDeviceSet->getInfo().Width, xDeviceGet->getInfo().Width);

    propName = "RecordChanges";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = "IsRecordChangesProtected";
    testBooleanReadonlyProperty(xSpreadsheetDocumentSettings, propName);

    propName = "Wildcards";
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
