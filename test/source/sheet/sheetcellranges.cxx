/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/sheetcellranges.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/propertyvalue.hxx>
#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void SheetCellRanges::testSheetCellRangesProperties()
{
    uno::Reference<beans::XPropertySet> xSheetCellRanges(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    uno::Sequence<beans::PropertyValue> aPropValue{ comphelper::makePropertyValue(
        u"StyleName"_ustr, u"Result2"_ustr) };

    propName = "ConditionalFormat";
    uno::Reference<sheet::XSheetConditionalEntries> aConditionalFormatGet;
    uno::Reference<sheet::XSheetConditionalEntries> aConditionalFormatSet;

    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ConditionalFormat",
                           xSheetCellRanges->getPropertyValue(propName) >>= aConditionalFormatGet);

    uno::Reference<sheet::XSheetConditionalEntries> aConditionalFormatNew(aConditionalFormatGet,
                                                                          UNO_SET_THROW);
    aConditionalFormatNew->addNew(aPropValue);

    aNewValue <<= aConditionalFormatNew;
    xSheetCellRanges->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCellRanges->getPropertyValue(propName) >>= aConditionalFormatSet);
    for (auto i = 0; i < aConditionalFormatSet->getCount(); i++)
    {
        uno::Reference<sheet::XSheetConditionalEntry> xSCENew(aConditionalFormatNew->getByIndex(i),
                                                              UNO_QUERY_THROW);
        uno::Reference<sheet::XSheetConditionalEntry> xSCESet(aConditionalFormatSet->getByIndex(i),
                                                              UNO_QUERY_THROW);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue ConditionalFormat["
                                         + std::to_string(i) + "]",
                                     xSCENew->getStyleName(), xSCESet->getStyleName());
    }

    propName = "ConditionalFormatLocal";
    uno::Reference<sheet::XSheetConditionalEntries> aConditionalFormatLocalGet;
    uno::Reference<sheet::XSheetConditionalEntries> aConditionalFormatLocalSet;

    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ConditionalFormatLocal",
                           xSheetCellRanges->getPropertyValue(propName)
                           >>= aConditionalFormatLocalGet);

    uno::Reference<sheet::XSheetConditionalEntries> aConditionalFormatLocalNew(
        aConditionalFormatLocalGet, UNO_SET_THROW);
    aConditionalFormatLocalNew->addNew(aPropValue);

    aNewValue <<= aConditionalFormatLocalNew;
    xSheetCellRanges->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCellRanges->getPropertyValue(propName) >>= aConditionalFormatLocalSet);
    for (auto i = 0; i < aConditionalFormatLocalSet->getCount(); i++)
    {
        uno::Reference<sheet::XSheetConditionalEntry> xSCENew(
            aConditionalFormatLocalNew->getByIndex(i), UNO_QUERY_THROW);
        uno::Reference<sheet::XSheetConditionalEntry> xSCESet(
            aConditionalFormatLocalSet->getByIndex(i), UNO_QUERY_THROW);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue ConditionalFormatLocal["
                                         + std::to_string(i) + "]",
                                     xSCENew->getStyleName(), xSCESet->getStyleName());
    }

    propName = "Validation";
    uno::Reference<beans::XPropertySet> aValidationGet;
    uno::Reference<beans::XPropertySet> aValidationSet;

    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Validation",
                           xSheetCellRanges->getPropertyValue(propName) >>= aValidationGet);

    uno::Reference<beans::XPropertySet> aValidationNew(aValidationGet, UNO_SET_THROW);
    uno::Any aValidationType;
    aValidationType <<= sheet::ValidationType_WHOLE;
    aValidationNew->setPropertyValue(u"Type"_ustr, aValidationType);

    aNewValue <<= aValidationNew;
    xSheetCellRanges->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCellRanges->getPropertyValue(propName) >>= aValidationSet);
    sheet::ValidationType aType;
    aValidationSet->getPropertyValue(u"Type"_ustr) >>= aType;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue Validation",
                                 sheet::ValidationType_WHOLE, aType);

    propName = "ValidationLocal";
    uno::Reference<beans::XPropertySet> aValidationLocalGet;
    uno::Reference<beans::XPropertySet> aValidationLocalSet;

    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ValidationLocal",
                           xSheetCellRanges->getPropertyValue(propName) >>= aValidationLocalGet);

    uno::Reference<beans::XPropertySet> aValidationLocalNew(aValidationLocalGet, UNO_SET_THROW);
    aValidationType <<= sheet::ValidationType_WHOLE;
    aValidationLocalNew->setPropertyValue(u"Type"_ustr, aValidationType);

    aNewValue <<= aValidationLocalNew;
    xSheetCellRanges->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCellRanges->getPropertyValue(propName) >>= aValidationLocalSet);
    aValidationLocalSet->getPropertyValue(u"Type"_ustr) >>= aType;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue ValidationLocal",
                                 sheet::ValidationType_WHOLE, aType);

    propName = "AbsoluteName";
    OUString aAbsoluteNameGet = u""_ustr;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue AbsoluteName",
                           xSheetCellRanges->getPropertyValue(propName) >>= aAbsoluteNameGet);

    OUString aAbsoluteNameSet = u"$Sheet1.$C$3"_ustr;
    aNewValue <<= aAbsoluteNameSet;
    xSheetCellRanges->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCellRanges->getPropertyValue(propName) >>= aAbsoluteNameSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Able to set PropertyValue AbsoluteName", aAbsoluteNameGet,
                                 aAbsoluteNameSet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
