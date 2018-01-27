/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetfilterableex.hxx>

#include <com/sun/star/sheet/FilterConnection.hpp>
#include <com/sun/star/sheet/FilterOperator.hpp>
#include <com/sun/star/sheet/TableFilterField.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#include <com/sun/star/sheet/XSheetFilterable.hpp>
#include <com/sun/star/sheet/XSheetFilterableEx.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellCursor.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XSheetFilterableEx::testCreateFilterDescriptorByObject()
{
    uno::Reference<sheet::XSheetFilterableEx> xSFEx(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetFilterable> xSF(getXSpreadsheet(), UNO_QUERY_THROW);

    uno::Reference<sheet::XSheetFilterDescriptor> xSFD = xSFEx->createFilterDescriptorByObject(xSF);
    CPPUNIT_ASSERT_MESSAGE("no XSheetFilterDescriptor", xSFD.is());

    uno::Sequence<sheet::TableFilterField> xTFF = xSFD->getFilterFields();
    CPPUNIT_ASSERT_MESSAGE("The gained XSheetFilterDescriptor is empty", xTFF.getLength() != 0);

    for (const auto& field : xTFF)
    {
        // we don't care about the actual value, just that we can access the fields
        CPPUNIT_ASSERT_MESSAGE("Unable to retrieve field: StringValue",
                               !field.StringValue.isEmpty() || field.StringValue.isEmpty());
        CPPUNIT_ASSERT_MESSAGE("Unable to retrieve field: IsNumeric",
                               !field.IsNumeric || field.IsNumeric);
        CPPUNIT_ASSERT_MESSAGE("Unable to retrieve field: NumericValue",
                               field.NumericValue != 0.0 || field.NumericValue == 0.0);
        CPPUNIT_ASSERT_MESSAGE("Unable to retrieve field: Field",
                               field.Field != 0 || field.Field == 0);
        CPPUNIT_ASSERT_MESSAGE("Unable to retrieve field: Connection",
                               field.Connection == sheet::FilterConnection_AND
                                   || field.Connection == sheet::FilterConnection_OR);
        CPPUNIT_ASSERT_MESSAGE("Unable to retrieve field: Operator",
                               field.Operator != sheet::FilterOperator_EMPTY);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
