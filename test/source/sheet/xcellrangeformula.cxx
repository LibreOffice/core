/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcellrangeformula.hxx>

#include <com/sun/star/sheet/XCellRangeFormula.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XCellRangeFormula::testGetSetFormulaArray()
{
    uno::Reference<sheet::XCellRangeFormula> xCRF(init(), UNO_QUERY_THROW);

    uno::Sequence<uno::Sequence<OUString>> aFormulaArrayOriginal = xCRF->getFormulaArray();
    uno::Sequence<uno::Sequence<OUString>> aFormulaArrayNew = xCRF->getFormulaArray();
    aFormulaArrayNew[0][0] = "NewValue";

    xCRF->setFormulaArray(aFormulaArrayNew);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get new FormulaArray", OUString("NewValue"),
                                 xCRF->getFormulaArray()[0][0]);

    xCRF->setFormulaArray(aFormulaArrayOriginal);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set new FormulaArray", aFormulaArrayOriginal[0][0],
                                 xCRF->getFormulaArray()[0][0]);
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
