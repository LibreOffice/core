/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <cpo/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace cpo::uno;

namespace apitest
{
void SpreadsheetDocumentSettings::testSpreadsheetDocumentSettingsProperties()
{
    uno::Reference<beans::XPropertySet> xSpreadsheetDocumentSettings(init(), UNO_QUERY_THROW);
    OUString propName;
    cpo::uno::Any aNewValue;

    propName = u"IsIterationEnabled"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"IterationCount"_ustr;
    testLongProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"IterationEpsilon"_ustr;
    testDoubleProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"StandardDecimals"_ustr;
    testShortProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"NullDate"_ustr;
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

    propName = u"DefaultTabStop"_ustr;
    testShortProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"IgnoreCase"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"CalcAsShown"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"MatchWholeCell"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"SpellOnline"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"LookUpLabels"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"RegularExpressions"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"ForbiddenCharacters"_ustr;
    uno::Reference<i18n::XForbiddenCharacters> xForbiddenCharactersGet;
    uno::Reference<i18n::XForbiddenCharacters> xForbiddenCharactersSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: ForbiddenCharacters",
                           xSpreadsheetDocumentSettings->getPropertyValue(propName)
                           >>= xForbiddenCharactersGet);

    lang::Locale aForbiddenCharactersLocale;
    aForbiddenCharactersLocale.Language = u"ru"_ustr;
    aForbiddenCharactersLocale.Country = u"RU"_ustr;

    uno::Reference<i18n::XForbiddenCharacters> xForbiddenCharactersNew;
    aNewValue <<= xForbiddenCharactersNew;
    xSpreadsheetDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetDocumentSettings->getPropertyValue(propName)
                   >>= xForbiddenCharactersSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Able to set PropertyValue: ForbiddenCharacters",
        xForbiddenCharactersGet->hasForbiddenCharacters(aForbiddenCharactersLocale),
        xForbiddenCharactersSet->hasForbiddenCharacters(aForbiddenCharactersLocale));

    propName = u"HasDrawPages"_ustr;
    testBooleanReadonlyProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"CharLocale"_ustr;
    lang::Locale aCharLocaleGet;
    lang::Locale aCharLocaleSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: CharLocale",
                           xSpreadsheetDocumentSettings->getPropertyValue(propName)
                           >>= aCharLocaleGet);

    lang::Locale aCharLocaleNew;
    aCharLocaleNew.Language = u"en"_ustr;
    aCharLocaleNew.Country = u"CA"_ustr;
    aNewValue <<= aCharLocaleNew;
    xSpreadsheetDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetDocumentSettings->getPropertyValue(propName) >>= aCharLocaleSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue: CharLocale", aCharLocaleNew.Country,
                                 aCharLocaleSet.Country);

    propName = u"CharLocaleAsian"_ustr;
    lang::Locale aCharLocaleAsianGet;
    lang::Locale aCharLocaleAsianSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: CharLocaleAsian",
                           xSpreadsheetDocumentSettings->getPropertyValue(propName)
                           >>= aCharLocaleAsianGet);

    lang::Locale aCharLocaleAsianNew;
    aCharLocaleAsianNew.Language = u"en"_ustr;
    aCharLocaleAsianNew.Country = u"CA"_ustr;
    aNewValue <<= aCharLocaleAsianNew;
    xSpreadsheetDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetDocumentSettings->getPropertyValue(propName)
                   >>= aCharLocaleAsianSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue: CharLocaleAsian",
                                 aCharLocaleAsianNew.Country, aCharLocaleAsianSet.Country);

    propName = u"CharLocaleComplex"_ustr;
    lang::Locale aCharLocaleComplexGet;
    lang::Locale aCharLocaleComplexSet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue: CharLocaleComplex",
                           xSpreadsheetDocumentSettings->getPropertyValue(propName)
                           >>= aCharLocaleComplexGet);

    lang::Locale aCharLocaleComplexNew;
    aCharLocaleComplexNew.Language = u"en"_ustr;
    aCharLocaleComplexNew.Country = u"CA"_ustr;
    aNewValue <<= aCharLocaleComplexNew;
    xSpreadsheetDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetDocumentSettings->getPropertyValue(propName)
                   >>= aCharLocaleComplexSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue: CharLocaleComplex",
                                 aCharLocaleComplexNew.Country, aCharLocaleComplexSet.Country);

    propName = u"IsLoaded"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"IsUndoEnabled"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"IsAdjustHeightEnabled"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"IsExecuteLinkEnabled"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"ReferenceDevice"_ustr;
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

    propName = u"RecordChanges"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"IsRecordChangesProtected"_ustr;
    testBooleanReadonlyProperty(xSpreadsheetDocumentSettings, propName);

    propName = u"Wildcards"_ustr;
    testBooleanProperty(xSpreadsheetDocumentSettings, propName);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
