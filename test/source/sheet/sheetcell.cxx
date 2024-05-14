/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/cppunitasserthelper.hxx>
#include <test/sheet/sheetcell.hxx>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/propertyvalue.hxx>
#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void SheetCell::testSheetCellProperties()
{
    uno::Reference<beans::XPropertySet> xSheetCell(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "Position";
    awt::Point aPositionGet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Position",
                           xSheetCell->getPropertyValue(propName) >>= aPositionGet);

    awt::Point aPositionSet(42, 42);
    aNewValue <<= aPositionSet;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aPositionSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Able to set PropertyValue Position", aPositionGet, aPositionGet);

    propName = "Size";
    awt::Size aSizeGet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue Size",
                           xSheetCell->getPropertyValue(propName) >>= aSizeGet);

    awt::Size aSizeSet(42, 42);
    aNewValue <<= aSizeGet;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aSizeSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Able to set PropertyValue Size", aSizeGet, aSizeSet);

    propName = "FormulaLocal";
    OUString aFormulaLocal = u""_ustr;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue FormulaLocal",
                           xSheetCell->getPropertyValue(propName) >>= aFormulaLocal);

    aNewValue <<= u"FormulaLocal"_ustr;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aFormulaLocal);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue FormulaLocal", u"FormulaLocal"_ustr,
                                 aFormulaLocal);

    propName = "CellContentType";
    table::CellContentType aCellContentTypeGet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue CellContentType",
                           xSheetCell->getPropertyValue(propName) >>= aCellContentTypeGet);

    table::CellContentType aCellContentTypeSet = table::CellContentType_TEXT;
    aNewValue <<= aCellContentTypeSet;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aCellContentTypeSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Able to set PropertyValue CellContentType", aCellContentTypeGet,
                                 aCellContentTypeSet);

    // Special case, because the API returns the wrong type.
    propName = "FormulaResultType";
    table::CellContentType aFormulaResultTypeGet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue FormulaResultType",
                           xSheetCell->getPropertyValue(propName) >>= aFormulaResultTypeGet);

    table::CellContentType aFormulaResultTypeSet = table::CellContentType_TEXT;
    aNewValue <<= aFormulaResultTypeSet;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aFormulaResultTypeSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Able to set PropertyValue FormulaResultType",
                                 aFormulaResultTypeGet, aFormulaResultTypeSet);

    propName = "FormulaResultType2";
    sal_Int32 aFormulaResultType2Get = 42;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue FormulaResultType2",
                           xSheetCell->getPropertyValue(propName) >>= aFormulaResultType2Get);

    sal_Int32 aFormulaResultType2Set = 42;
    aNewValue <<= aFormulaResultType2Set;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aFormulaResultType2Set);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Able to set PropertyValue FormulaResultType2",
                                 aFormulaResultType2Get, aFormulaResultType2Set);

    uno::Sequence<beans::PropertyValue> aPropValue{ comphelper::makePropertyValue(
        u"StyleName"_ustr, u"Result2"_ustr) };

    propName = "ConditionalFormat";
    uno::Reference<sheet::XSheetConditionalEntries> aConditionalFormatGet;
    uno::Reference<sheet::XSheetConditionalEntries> aConditionalFormatSet;

    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ConditionalFormat",
                           xSheetCell->getPropertyValue(propName) >>= aConditionalFormatGet);

    uno::Reference<sheet::XSheetConditionalEntries> aConditionalFormatNew(aConditionalFormatGet,
                                                                          UNO_SET_THROW);
    aConditionalFormatNew->addNew(aPropValue);

    aNewValue <<= aConditionalFormatNew;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aConditionalFormatSet);
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
                           xSheetCell->getPropertyValue(propName) >>= aConditionalFormatLocalGet);

    uno::Reference<sheet::XSheetConditionalEntries> aConditionalFormatLocalNew(
        aConditionalFormatLocalGet, UNO_SET_THROW);
    aConditionalFormatLocalNew->addNew(aPropValue);

    aNewValue <<= aConditionalFormatLocalNew;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aConditionalFormatLocalSet);
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
                           xSheetCell->getPropertyValue(propName) >>= aValidationGet);

    uno::Reference<beans::XPropertySet> aValidationNew(aValidationGet, UNO_SET_THROW);
    uno::Any aValidationType;
    aValidationType <<= sheet::ValidationType_WHOLE;
    aValidationNew->setPropertyValue(u"Type"_ustr, aValidationType);

    aNewValue <<= aValidationNew;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aValidationSet);
    sheet::ValidationType aType;
    aValidationSet->getPropertyValue(u"Type"_ustr) >>= aType;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue Validation",
                                 sheet::ValidationType_WHOLE, aType);

    propName = "ValidationLocal";
    uno::Reference<beans::XPropertySet> aValidationLocalGet;
    uno::Reference<beans::XPropertySet> aValidationLocalSet;

    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ValidationLocal",
                           xSheetCell->getPropertyValue(propName) >>= aValidationLocalGet);

    uno::Reference<beans::XPropertySet> aValidationLocalNew(aValidationLocalGet, UNO_SET_THROW);
    aValidationType <<= sheet::ValidationType_WHOLE;
    aValidationLocalNew->setPropertyValue(u"Type"_ustr, aValidationType);

    aNewValue <<= aValidationLocalNew;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aValidationLocalSet);
    aValidationLocalSet->getPropertyValue(u"Type"_ustr) >>= aType;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue ValidationLocal",
                                 sheet::ValidationType_WHOLE, aType);

    propName = "AbsoluteName";
    OUString aAbsoluteNameGet = u""_ustr;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue AbsoluteName",
                           xSheetCell->getPropertyValue(propName) >>= aAbsoluteNameGet);

    OUString aAbsoluteNameSet = u"$Sheet1.$C$3"_ustr;
    aNewValue <<= aAbsoluteNameSet;
    xSheetCell->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSheetCell->getPropertyValue(propName) >>= aAbsoluteNameSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Able to set PropertyValue AbsoluteName", aAbsoluteNameGet,
                                 aAbsoluteNameSet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
