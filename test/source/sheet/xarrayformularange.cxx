/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xarrayformularange.hxx>

#include <com/sun/star/sheet/XArrayFormulaRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XArrayFormulaRange::testGetSetArrayFormula()
{
    uno::Reference<sheet::XArrayFormulaRange> xAFR(init(), UNO_QUERY_THROW);

    static const OUStringLiteral sFormula(u"=1 + 2 * 5");

    xAFR->setArrayFormula(sFormula);
    uno::Reference<sheet::XCellRangeAddressable> xCRA(xAFR, UNO_QUERY_THROW);
    table::CellRangeAddress cellRangeAddress = xCRA->getRangeAddress();

    uno::Reference<sheet::XSpreadsheet> xSheet(getXSpreadsheet(), UNO_QUERY_THROW);

    for (auto i = cellRangeAddress.StartColumn; i <= cellRangeAddress.EndColumn; i++)
    {
        for (auto j = cellRangeAddress.StartRow; j <= cellRangeAddress.EndRow; j++)
        {
            uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(i, j);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Value of Cell " + std::to_string(i) + ","
                                                     + std::to_string(j),
                                                 11.0, xCell->getValue(), 0.5);
        }
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to test getArrayFormula()", OUString("{" + sFormula + "}"),
                                 xAFR->getArrayFormula());
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
