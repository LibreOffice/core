/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetfilterable.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/FilterOperator.hpp>
#include <com/sun/star/sheet/TableFilterField.hpp>
#include <com/sun/star/sheet/XSheetFilterable.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest {

void XSheetFilterable::testCreateFilterDescriptor()
{
    uno::Reference< sheet::XSheetFilterable > xFA(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetFilterDescriptor > xSFD = xFA->createFilterDescriptor(true);

    uno::Sequence< sheet::TableFilterField > xTFF{
        { /* Connection   */ {},
          /* Field        */ 0,
          /* Operator     */ sheet::FilterOperator_GREATER_EQUAL,
          /* IsNumeric    */ true,
          /* NumericValue */ 2,
          /* StringValue  */ {}},
        { /* Connection   */ {},
          /* Field        */ 1,
          /* Operator     */ sheet::FilterOperator_LESS,
          /* IsNumeric    */ false,
          /* NumericValue */ {},
          /* StringValue  */ u"C"_ustr }
    };

    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Unable to create XSheetFilterDescriptor", xSFD->setFilterFields(xTFF));
}

void XSheetFilterable::testFilter()
{
    uno::Reference< sheet::XSpreadsheet > xSheet(getXSpreadsheet(), UNO_QUERY_THROW);

    uno::Reference< sheet::XSheetFilterable > xFA(xSheet, UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetFilterDescriptor > xSFD = xFA->createFilterDescriptor(true);

    uno::Sequence< sheet::TableFilterField > xTFF{
        { /* Connection   */ {},
          /* Field        */ 0,
          /* Operator     */ sheet::FilterOperator_GREATER_EQUAL,
          /* IsNumeric    */ true,
          /* NumericValue */ 2,
          /* StringValue  */ {}},
        { /* Connection   */ {},
          /* Field        */ 1,
          /* Operator     */ sheet::FilterOperator_LESS,
          /* IsNumeric    */ false,
          /* NumericValue */ {},
          /* StringValue  */ u"C"_ustr }
    };
    xSFD->setFilterFields(xTFF);

    xSheet->getCellByPosition(0, 0)->setValue(1);
    xSheet->getCellByPosition(0, 1)->setValue(2);
    xSheet->getCellByPosition(0, 2)->setValue(3);
    xSheet->getCellByPosition(1, 0)->setFormula(u"A"_ustr);
    xSheet->getCellByPosition(1, 1)->setFormula(u"B"_ustr);
    xSheet->getCellByPosition(1, 2)->setFormula(u"C"_ustr);
    xFA->filter(xSFD);

    uno::Reference< table::XColumnRowRange > xColRowRange(xSheet, UNO_QUERY_THROW);
    uno::Reference< table::XTableRows > xTableRows = xColRowRange->getRows();
    uno::Reference< beans::XPropertySet > xRowProps(xTableRows->getByIndex(0), UNO_QUERY_THROW);
    bool bIsVisible = true;
    CPPUNIT_ASSERT(xRowProps->getPropertyValue(u"IsVisible"_ustr) >>= bIsVisible);

    CPPUNIT_ASSERT_MESSAGE("Row 1 should be invisible", !bIsVisible);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
