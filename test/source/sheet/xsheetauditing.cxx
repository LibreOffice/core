/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetauditing.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/sheet/XSheetAuditing.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XSheetAuditing::testShowHideDependents()
{
    uno::Reference<sheet::XSheetAuditing> xAuditing(init(), UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPageSupplier> xDPS(xAuditing, UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDPS->getDrawPage();
    const sal_Int32 nElements = xDrawPage->getCount();

    xAuditing->showDependents(table::CellAddress(0, 8, 6));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to showDependents()", nElements + 1,
                                 xDrawPage->getCount());

    xAuditing->hideDependents(table::CellAddress(0, 8, 6));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to hideDependents()", nElements, xDrawPage->getCount());
}

void XSheetAuditing::testShowHidePrecedents()
{
    uno::Reference<sheet::XSheetAuditing> xAuditing(init(), UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPageSupplier> xDPS(xAuditing, UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDPS->getDrawPage();
    const sal_Int32 nElements = xDrawPage->getCount();

    xAuditing->showPrecedents(table::CellAddress(0, 8, 6));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to showPrecedents()", nElements + 2,
                                 xDrawPage->getCount());

    xAuditing->hidePrecedents(table::CellAddress(0, 8, 6));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to showPrecedents()", nElements, xDrawPage->getCount());
}

void XSheetAuditing::testClearArrows()
{
    uno::Reference<sheet::XSheetAuditing> xAuditing(init(), UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPageSupplier> xDPS(xAuditing, UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDPS->getDrawPage();
    const sal_Int32 nElements = xDrawPage->getCount();

    xAuditing->showPrecedents(table::CellAddress(0, 8, 6));
    xAuditing->showDependents(table::CellAddress(0, 8, 6));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set arrows", nElements + 3, xDrawPage->getCount());

    xAuditing->clearArrows();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to clear arrows", nElements, xDrawPage->getCount());
}

void XSheetAuditing::testShowErrors()
{
    uno::Reference<sheet::XSheetAuditing> xAuditing(init(), UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPageSupplier> xDPS(xAuditing, UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDPS->getDrawPage();
    const sal_Int32 nElements = xDrawPage->getCount();

    uno::Reference<sheet::XSpreadsheet> xSheet(xAuditing, UNO_QUERY_THROW);
    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(7, 6);
    xCell->setValue(-9);
    xCell->setFormula("=SQRT(" + OUStringChar(static_cast<char>('A' + 7)) + OUString::number(7)
                      + ")");

    uno::Reference<text::XText> xText(xCell, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No error code", u"Err:522"_ustr, xText->getString());

    xAuditing->showErrors(table::CellAddress(0, 7, 6));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to show errors", nElements + 1, xDrawPage->getCount());
}

void XSheetAuditing::testShowInvalid()
{
    uno::Reference<sheet::XSheetAuditing> xAuditing(init(), UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPageSupplier> xDPS(xAuditing, UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDPS->getDrawPage();
    const sal_Int32 nElements = xDrawPage->getCount();

    uno::Reference<sheet::XSpreadsheet> xSheet(xAuditing, UNO_QUERY_THROW);
    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(7, 6);
    xCell->setValue(2.5);

    uno::Reference<beans::XPropertySet> xPropSet(xCell, UNO_QUERY_THROW);
    uno::Any aValidation = xPropSet->getPropertyValue(u"Validation"_ustr);
    uno::Reference<beans::XPropertySet> xValidation(aValidation, UNO_QUERY_THROW);
    uno::Any aAny;

    aAny <<= sheet::ValidationType_WHOLE;
    xValidation->setPropertyValue(u"Type"_ustr, aAny);
    aAny <<= xValidation;
    xPropSet->setPropertyValue(u"Validation"_ustr, aAny);
    xAuditing->showInvalid();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to show invalid (WHOLE)", nElements + 1,
                                 xDrawPage->getCount());

    xAuditing->clearArrows();

    aAny <<= sheet::ValidationType_ANY;
    xValidation->setPropertyValue(u"Type"_ustr, aAny);
    aAny <<= xValidation;
    xPropSet->setPropertyValue(u"Validation"_ustr, aAny);

    xAuditing->showInvalid();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to show invalid (ANY)", nElements, xDrawPage->getCount());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
